all:
	@echo "> Wrong arguments for make."
	@echo "> make <mandel / bridge / train>"
mandel: ccr_mandelGUI.c mandelCore.c mandelCore.h mtx_cond.c mtx_cond.h ccr.h
	gcc ccr_mandelGUI.c mandelCore.c mtx_cond.c -o ccr_mandelGUI -L /usr/X11R6/lib -lX11 -lpthread

bridge:	ccr_bridge.c mtx_cond.c mtx_cond.h ccr.h
	gcc -Wall -g ccr_bridge.c mtx_cond.c -o ccr_bridge -lpthread

train: ccr_train.c mtx_cond.c mtx_cond.h ccr.h
	gcc -Wall -g ccr_train.c mtx_cond.c -o ccr_train -lpthread

clean:
		find . -type f | xargs touch
		rm -rf $(OBJS)
