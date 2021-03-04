
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "proto.h"
#include "string.h"
#include "proc.h"
#include "global.h"

#define F_COLOR	0x07//黑底白字 NORMAL F


SEMAPHORE rmutex;
SEMAPHORE wmutex;
SEMAPHORE S;
SEMAPHORE maxReaderMutex;
int readcount;
int mode;
int writecount;
SEMAPHORE x;
SEMAPHORE y;
SEMAPHORE z;

/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	//for (p = proc_table; p < proc_table+NR_TASKS; p++) {
	//	if(p -> sleep > 0){
	//		p -> sleep--;//减掉睡眠的时间
	//	}
	//}

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if (p -> wait>0 || p -> sleep>0){
				continue;//若在等待状态或有睡眠时间，就不分配时间片
			}
			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			if (p -> wait>0 || p -> sleep>0){
				continue;//若在等待状态或有睡眠时间，就不分配时间片
			}
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				p->ticks = p->priority;
			}
		}
	}
}
/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks(){
	return ticks;
}
PUBLIC int sys_process_sleep(int milli_seconds){
	p_proc_ready->sleep=milli_seconds/10;
	schedule();
	return 0;
}
PUBLIC void sys_print_color(char* str,int color){
	disp_color_str(str,color);
	disp_color_str("",color);
	disp_str("");
}
PUBLIC int sys_P(SEMAPHORE* s){
	s->value--;
	//disp_int(s->value);
	if(s->value<0){
		p_proc_ready->wait=1;
		s->list[s->ptr]=p_proc_ready;
		s->ptr++;
		schedule();
	}
	// else{
	// 	sys_disp_color_str("have access to the file");
	// }
	return 0;
}
PUBLIC int sys_V(SEMAPHORE* s){
	s->value++;
	//disp_int(s->value);
	if(s->value<=0){
		s->list[0]->wait=0;
		for(int i=0;i<s->ptr;i++){
			s->list[i]=s->list[i+1];
		}
		s->ptr--;
	}
	return 0;
}

PUBLIC void init_semaphore(int readerNum){
	rmutex.value=1;
	wmutex.value=1;
	S.value=1;
	readcount=0;
	mode=2;
	maxReaderMutex.value=readerNum;
	//1 reader first
	//0 writer first
	//2 writer first first first
	x.value=1;
	y.value=1;
	z.value=1;
	writecount=0;
}


PUBLIC void reader(char* name,int len,int color){
	char want[16]=" wants to read.";
	char is[13]=" is reading.";
	char fin[19]=" finishes reading.";
	if(mode==1){ //读者优先 1  reader
		sys_disp_color_str(name,color);
		sys_disp_color_str(want,color);
		
		sys_p(&maxReaderMutex);
		sys_p(&rmutex);
		if(readcount==0) sys_p(&wmutex);
		//disp_int(wmutex.value);
		readcount++;
		//disp_int(readcount);
		sys_v(&rmutex);

		
		//read the file.
		sys_disp_color_str(name,color);
		sys_disp_color_str(is,color);
		//sys_sleep(len);
		milli_delay(len);
		sys_disp_color_str(name,color);
		sys_disp_color_str(fin,color);
		

		sys_p(&rmutex);
		readcount--;
		if(readcount==0) sys_v(&wmutex);
		sys_v(&rmutex);
		sys_v(&maxReaderMutex);

		//sys_sleep(1000);
	}else if(mode==0){//写者优先0  reader
		sys_disp_color_str(name,color);
		sys_disp_color_str(want,color);
		sys_p(&maxReaderMutex);
		sys_p(&S);
		sys_p(&rmutex);
		if(readcount==0) sys_p(&wmutex);
		readcount++;
		sys_v(&rmutex);
		sys_v(&S);

		
		//read the file.
		sys_disp_color_str(name,color);
		sys_disp_color_str(is,color);
		milli_delay(len);
		sys_disp_color_str(name,color);
		sys_disp_color_str(fin,color);
		

		sys_p(&rmutex);
		readcount--;
		if(readcount==0) sys_v(&wmutex);
		sys_v(&rmutex);
		sys_v(&maxReaderMutex);
		//sys_sleep(1000);
	}else if(mode==2){//读者超优先 2 reader
		sys_disp_color_str(name,color);
		sys_disp_color_str(want,color);
		sys_p(&maxReaderMutex);
		sys_p(&z);
		sys_p(&rmutex);
		sys_p(&x);
		readcount++;
		if(readcount==1) sys_p(&wmutex);
		sys_v(&x);
		sys_v(&rmutex);
		sys_v(&z);
		//read the file.
		sys_disp_color_str(name,color);
		sys_disp_color_str(is,color);
		milli_delay(len);
		sys_disp_color_str(name,color);
		sys_disp_color_str(fin,color);
		sys_p(&x);
		readcount--;
		if(readcount==0) sys_v(&wmutex);
		sys_v(&x);
		sys_v(&maxReaderMutex);
		sys_sleep(1000);
	}
}
PUBLIC void writer(char* name,int len,int color){
	char want[17]=" wants to write.";
	char is[13]=" is writing.";
	char fin[19]=" finishes writing.";
	if(mode==1){ //读者优先 1   writer
		sys_disp_color_str(name,color);
		sys_disp_color_str(want,color);
		sys_p(&wmutex);
		//disp_int(wmutex.value);
		//write the file.
		sys_disp_color_str(name,color);
		sys_disp_color_str(is,color);
		milli_delay(len);
		sys_disp_color_str(name,color);
		sys_disp_color_str(fin,color);
		sys_v(&wmutex);
		//sys_sleep(1000);
	}else if(mode==0){//写者优先 0 writer
		sys_disp_color_str(name,color);
		sys_disp_color_str(want,color);
		sys_p(&S);
		sys_p(&wmutex);
		//write the file.
		sys_disp_color_str(name,color);
		sys_disp_color_str(is,color);
		milli_delay(len);
		sys_disp_color_str(name,color);
		sys_disp_color_str(fin,color);
		sys_v(&wmutex);
		sys_v(&S);
		//sys_sleep(1000);
	}else if(mode==2){//写者超优先 2 writer
		sys_disp_color_str(name,color);
		sys_disp_color_str(want,color);
		sys_p(&y);
		writecount++;
		if(writecount==1) sys_p(&rmutex);
		sys_v(&y);
		sys_p(&wmutex);
		//write the file.
		sys_disp_color_str(name,color);
		sys_disp_color_str(is,color);
		milli_delay(len);
		sys_disp_color_str(name,color);
		sys_disp_color_str(fin,color);
		sys_v(&wmutex);
		sys_p(&y);
		writecount--;
		if(writecount==0) sys_v(&rmutex);
		sys_v(&y);
		sys_sleep(1000);
	}
}


PUBLIC void report(){
	if(readcount==1){
		char m1[16]="ONE IS READING.";
		sys_disp_color_str(m1,F_COLOR);
	}else if(readcount==2){
		char m2[17]="TWO ARE READING.";
		sys_disp_color_str(m2,F_COLOR);
	}else if(readcount==3){
		char m3[19]="THREE ARE READING.";
		sys_disp_color_str(m3,F_COLOR);
	}else if(readcount==0) {
		char m4[16]="ONE IS WRITING.";
		sys_disp_color_str(m4,F_COLOR);
	}else{
		char err[20]="READCOUNT IS ERROR.";
		sys_disp_color_str(err,F_COLOR);
	}
}
