/* Axelou Olympia, 2161
 * Tsitsopoulou Eirini, 2203
 *
 * We changed the original program so that the mandelbrot computation function is made in parallel
 * Every worker thread computes a slice of the final image and the main thread draws the result
 *
 * The synchronization and the communication betweeen main thread and workers
 * are dessigned with mutexes and conditions.
 */

#include "mtx_cond.h"
#include "mandelCore.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>

volatile int *res, *draw_array;
volatile mandel_Pars * slices;
volatile int maxIterations, threadycast, nofslices;
volatile int nofjustfin, main_assign_w, main_draw_w;

pthread_mutex_t mtx;
pthread_cond_t cond_args;
pthread_cond_t cond_assign;
pthread_cond_t cond_m_assign;
pthread_cond_t cond_draw;

#define JUST_FINISHED 1

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
  int my_no = *(int *)arg;
  int mtx_res;

  //be sure to take the right args
  cond_signal(&cond_args, __LINE__);

  while(1){
    // wait for main to assign job
    mtx_lock(&mtx, __LINE__);
    threadycast++;
    printf("%d: threadycast = %d\n", my_no, threadycast);
    if(threadycast == nofslices){
      printf("%d: notifying main: everyone is here and waiting for the job\n", my_no);
      if(main_assign_w){
        cond_signal(&cond_m_assign, __LINE__);
        main_assign_w = 0;
      }
    }
    // printf("worker %d: going to block\n", my_no);

    cond_wait(&cond_assign, &mtx, __LINE__); // in CS to make sure that main doesn't "signal" before it "wait"s ?
    // printf("worker %d: starting working\n", my_no);
    mtx_unlock(&mtx, __LINE__);

    // perform the Mandelbrot computation
    mandel_Calc((mandel_Pars*)slices+my_no,maxIterations,(int*)res + my_no*slices[my_no].imSteps*slices[my_no].reSteps);

    //notify main
    mtx_lock(&mtx, __LINE__);
    draw_array[my_no] = JUST_FINISHED;
    nofjustfin++;
    if(main_draw_w){
      cond_signal(&cond_draw, __LINE__);
      main_draw_w = 0;
    }
    mtx_unlock(&mtx, __LINE__);

  }
  return NULL;
}


int main(int argc, char *argv[]) {
  mandel_Pars pars;
  int i,j,x,y,level;
  int xoff,yoff;
  long double reEnd,imEnd,reCenter,imCenter;
  pthread_t *pth_array;
  pthread_mutexattr_t mtx_attr;
  pthread_condattr_t cond_attr;
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

  //allocating volatile arrays: slices, res
  //the pthread_t array for pthread_create
  //and int draw_array for drawing synchronization
  slices = (volatile mandel_Pars *) malloc(sizeof(mandel_Pars)*nofslices);
  res = (volatile int *) malloc(sizeof(int)*pars.reSteps*pars.imSteps);
  pth_array = (pthread_t*) malloc(sizeof(pthread_t)*nofslices);
  draw_array = (volatile int *) malloc(sizeof(int)*nofslices);

  //initialising mutex
  mtx_init(&mtx, __LINE__);

  //initialising conditions
  threadycast = 0;
  main_draw_w = 0;
  main_assign_w = 0;
  cond_init(&cond_args, __LINE__);
  cond_init(&cond_assign, __LINE__);
  cond_init(&cond_m_assign, __LINE__);
  cond_init(&cond_draw, __LINE__);

  // creating threads
  for (i=0; i<nofslices; i++){
      mtx_lock(&mtx, __LINE__); // to lock xreiazetai na einai edw ?
      if (pthread_create(&pth_array[i], NULL, worker, (void*)&i))
          printf("error at pthread_create no: %d\n", i);

      // waits for the thread to take its argument
      // before continuing with the next one
      cond_wait(&cond_args, &mtx, __LINE__);
      mtx_unlock(&mtx, __LINE__);
  }

// ************************* END OF INITIALISATIONS *****************************

  /* open window for drawing results */

  openDisplay();
  openWin(argv[0], WinW, WinH);

  level = 1;

  while (1) {

    clearWin();
    // create N jobs and assign them to workers
    mandel_Slice(&pars, nofslices, (mandel_Pars *) slices);

    mtx_lock(&mtx, __LINE__);
    // printf("main: going to wait\n");
    if(threadycast < nofslices){  //ERWTHSH: MPOROUME NA XRISIMOPOIHSOYME BROADCAST
      main_assign_w = 1;
      cond_wait(&cond_m_assign, &mtx, __LINE__);
    }
    // printf("main: ready to notify workers\n");

    threadycast = 0; // initialising for next computation
    mtx_unlock(&mtx, __LINE__);

    // notify workers
    for (i = nofslices - 1; i > -1; i--){
      //printf("main: i = %d signal\n", i);
      cond_signal(&cond_assign, __LINE__);
    }

    y=0;
    workersDone = 0;
    while (1){ // while not all workers done

      mtx_lock(&mtx, __LINE__);
      // if no one has finished, main blocks
      if (nofjustfin == 0){
        main_draw_w = 1; // exactly like the barber, right?!?!?!?! :D
        cond_wait(&cond_draw, &mtx, __LINE__);
      }

      //for: breaks when it finds a worker which has just finished
      for(i = 0; i < nofslices; i++){
        // printf("i = %d\n",i);
        if(draw_array[i] == JUST_FINISHED){
          draw_array[i] = !JUST_FINISHED;
          nofjustfin--;

          workersDone++;

          printf("Main: printing %d\n", i);
          break;
        }
      }

      mtx_unlock(&mtx, __LINE__);

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
      // checks if everyone has finished, if so, breaks out of while
      if(workersDone == nofslices)
          break;
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
    printf("* * * * * * NEW COORDINATES GIVEN * * * * * *\n");
  }

  /* never reach this point; for cosmetic reasons */

  free((mandel_Pars *)slices);
  free((int *)res);

  closeWin();
  closeDisplay();
}
