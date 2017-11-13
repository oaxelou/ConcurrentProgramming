/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * We changed the original program so that the mandelbrot computation function is made in parallel
 * Every worker thread computes a slice of the final image and the main thread draws the result
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "mandelCore.h"

#define MAIN_WAITING 4
#define TH_READY 3
#define WORKING 2
#define JUST_FINISHED 1
#define NOT_WORKING 0

volatile int *res;
volatile mandel_Pars * slices;
volatile int *sync_array, maxIterations;

#define WinW 300
#define WinH 300
#define ZoomStepFactor 0.5
#define ZoomIterationFactor 2

static Display *dsp = NULL;
static unsigned long curC;
static Window win;
static GC gc;

/* basic win management rountines */

static void openDisplay() {
  if (dsp == NULL) {
    dsp = XOpenDisplay(NULL);
  }
}

static void closeDisplay() {
  if (dsp != NULL) {
    XCloseDisplay(dsp);
    dsp=NULL;
  }
}

void openWin(const char *title, int width, int height) {
  unsigned long blackC,whiteC;
  XSizeHints sh;
  XEvent evt;
  long evtmsk;

  whiteC = WhitePixel(dsp, DefaultScreen(dsp));
  blackC = BlackPixel(dsp, DefaultScreen(dsp));
  curC = blackC;

  win = XCreateSimpleWindow(dsp, DefaultRootWindow(dsp), 0, 0, WinW, WinH, 0, blackC, whiteC);

  sh.flags=PSize|PMinSize|PMaxSize;
  sh.width=sh.min_width=sh.max_width=WinW;
  sh.height=sh.min_height=sh.max_height=WinH;
  XSetStandardProperties(dsp, win, title, title, None, NULL, 0, &sh);

  XSelectInput(dsp, win, StructureNotifyMask|KeyPressMask);
  XMapWindow(dsp, win);
  do {
    XWindowEvent(dsp, win, StructureNotifyMask, &evt);
  } while (evt.type != MapNotify);

  gc = XCreateGC(dsp, win, 0, NULL);

}

void closeWin() {
  XFreeGC(dsp, gc);
  XUnmapWindow(dsp, win);
  XDestroyWindow(dsp, win);
}

void flushDrawOps() {
  XFlush(dsp);
}

void clearWin() {
  XSetForeground(dsp, gc, WhitePixel(dsp, DefaultScreen(dsp)));
  XFillRectangle(dsp, win, gc, 0, 0, WinW, WinH);
  flushDrawOps();
  XSetForeground(dsp, gc, curC);
}

void drawPoint(int x, int y) {
  XDrawPoint(dsp, win, gc, x, WinH-y);
  flushDrawOps();
}

void getMouseCoords(int *x, int *y) {
  XEvent evt;

  XSelectInput(dsp, win, ButtonPressMask);
  do {
    XNextEvent(dsp, &evt);
  } while (evt.type != ButtonPress);
  *x=evt.xbutton.x; *y=evt.xbutton.y;
}

/* color stuff */

void setColor(char *name) {
  XColor clr1,clr2;

  if (!XAllocNamedColor(dsp, DefaultColormap(dsp, DefaultScreen(dsp)), name, &clr1, &clr2)) {
    printf("failed\n"); return;
  }
  XSetForeground(dsp, gc, clr1.pixel);
  curC = clr1.pixel;
}

char *pickColor(int v, int maxIterations) {
  static char cname[128];

  if (v == maxIterations) {
    return("black");
  }
  else {
    sprintf(cname,"rgb:%x/%x/%x",v%64,v%128,v%256);
    return(cname);
  }
}

/* The thread function that performs the Mandelbrot computation
 *
 * Input: the number of the thread as argument
 */
void *worker (void *arg){
  int my_join = *(int *)arg;

  sync_array[my_join] = TH_READY;

  while(1){
    // wait for main to assign job
    while(sync_array[my_join] != WORKING);

    // perform the Mandelbrot computation
    mandel_Calc((mandel_Pars*)slices+my_join,maxIterations,(int*)res + my_join*slices[my_join].imSteps*slices[my_join].reSteps);

    //notify main
    sync_array[my_join] = JUST_FINISHED;
  }
  return NULL;
}


int main(int argc, char *argv[]) {
  mandel_Pars pars;
  int i,j,x,y,nofslices,level;
  int xoff,yoff;
  long double reEnd,imEnd,reCenter,imCenter;
  pthread_t *pth_array;
  int workersDone;

  printf("\n");
  printf("This program starts by drawing the default Mandelbrot region\n");
  printf("When done, you can click with the mouse on an area of interest\n");
  printf("and the program will automatically zoom around this point\n");
  printf("\n");
  printf("Press enter to continue\n");
  getchar();

  pars.reSteps = WinW; /* never changes */
  pars.imSteps = WinH; /* never changes */

  /* default mandelbrot region */

  pars.reBeg = (long double) -2.0;
  reEnd = (long double) 1.0;
  pars.imBeg = (long double) -1.5;
  imEnd = (long double) 1.5;
  pars.reInc = (reEnd - pars.reBeg) / pars.reSteps;
  pars.imInc = (imEnd - pars.imBeg) / pars.imSteps;

  printf("enter max iterations (50): ");
  scanf("%d",&maxIterations);
  printf("enter no of slices: ");
  scanf("%d",&nofslices);

  /* adjust slices to divide win height */

  while (WinH % nofslices != 0) { nofslices++;}

  /* allocate slice parameter and result arrays */

  //allocating volatile arrays: slices, res, sync_array
  //and the pthread_t array for pthread_create
  slices = (volatile mandel_Pars *) malloc(sizeof(mandel_Pars)*nofslices);
  res = (volatile int *) malloc(sizeof(int)*pars.reSteps*pars.imSteps);
  pth_array = (pthread_t*) malloc(sizeof(pthread_t)*nofslices);
  sync_array = (volatile int *)malloc(sizeof(int)*nofslices);

  // initialising sync_array
  for (i=0; i<nofslices; i++)
      sync_array[i] = MAIN_WAITING;

  // creating threads
  for (i=0; i<nofslices; i++){
      if (pthread_create(&pth_array[i], NULL, worker, (void*)&i))
          printf("error at pthread_create no: %d\n", i);

      // waits for the thread to take its argument
      // before continuing with the next one
      while (sync_array[i] == MAIN_WAITING){}
  }


  /* open window for drawing results */

  openDisplay();
  openWin(argv[0], WinW, WinH);

  level = 1;

  while (1) {

    clearWin();
    // create N jobs and assign them to workers
    mandel_Slice(&pars,nofslices, (mandel_Pars *) slices);

    // notify workers
    for (i = nofslices - 1; i > -1; i--){
      sync_array[i] = WORKING;
    }

    y=0;
    while (1){ // while not all workers done

        //for: runs through sync_array, breaks when it finds a thread which has just_finished
        workersDone = 0;
        for(i = 0; i < nofslices; i++){
            if(sync_array[i] == JUST_FINISHED){
                sync_array[i] = NOT_WORKING;
                break;
            }
            //if i thread is not working, increases counter of non-working threads
            if(sync_array[i] == NOT_WORKING)
                workersDone++;
        }
        // checks if everyone has finished, if so, breaks out of while
        if(workersDone == nofslices)
            break;

        //when for-break occurs (a thread has just finished), it draws the result
        if(i < nofslices){
          y = i*slices[i].imSteps;
          for (j=0; j<slices[i].imSteps; j++) {
            for (x=0; x<slices[i].reSteps; x++) {
              setColor(pickColor(res[y*slices[i].reSteps+x],maxIterations));
              drawPoint(x, y);
            }
            y++;
        }
      }
    }


    /* get next focus/zoom point */

    getMouseCoords(&x,&y);
    xoff = x;
    yoff = WinH-y;

    /* adjust region and zoom factor  */

    reCenter = pars.reBeg + xoff*pars.reInc;
    imCenter = pars.imBeg + yoff*pars.imInc;
    pars.reInc = pars.reInc*ZoomStepFactor;
    pars.imInc = pars.imInc*ZoomStepFactor;
    pars.reBeg = reCenter - (WinW/2)*pars.reInc;
    pars.imBeg = imCenter - (WinH/2)*pars.imInc;

    maxIterations = maxIterations*ZoomIterationFactor;
    level++;

  }

  /* never reach this point; for cosmetic reasons */

  free((mandel_Pars *)slices);
  free((int *)res);

  closeWin();
  closeDisplay();

}
