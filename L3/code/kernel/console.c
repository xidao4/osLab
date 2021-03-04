
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
*/


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);
PRIVATE void exitFIND(CONSOLE* p_con);
PRIVATE void findStrAndShow();
PRIVATE void setRedStr(u8* p_str_start);

PRIVATE u32 start;
PRIVATE u32 end;
PRIVATE u32 onlyESC=0;
PRIVATE u32 len;

/*======================================================================*
			   init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
	int nr_tty = p_tty - tty_table;
	p_tty->p_console = console_table + nr_tty;

	int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */

	int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
	p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
	p_tty->p_console->v_mem_limit        = con_v_mem_size;
	p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;

	/* 默认光标位置在最开始处 */
	//p_tty->p_console->cursor = p_tty->p_console->original_addr;
	//p_tty->p_console->cursor = disp_pos / 2;
	//set_cursor(p_tty->p_console->cursor);
	disp_pos = 0;
	for (int i = 0; i < 80 * 25; i++) {
		disp_str(" ");
	}
	disp_pos = 0;
	p_tty->p_console->cursor = p_tty->p_console->original_addr;
	set_cursor(p_tty->p_console->cursor);
	//p_tty->p_console->cursor = p_tty->p_console->original_addr;
	//set_cursor(p_tty->p_console->cursor);*/
	//if (nr_tty == 0) {
	//	/* 第一个控制台沿用原来的光标位置 */
	//	p_tty->p_console->cursor = disp_pos / 2;
	//	disp_pos = 0;
	//	set_cursor(p_tty->p_console->cursor);
	//	/*for (int i = 0; i < 80 * 25; i++) {
	//		out_char(p_tty->p_console, '\b');
	//	}*/
	//	/* 通过打印空格的方式清空屏幕的前五行，并把 disp_pos 清零 */
	//	for (int i = 0; i < 80 * 5; i++) {
	//		disp_str(" ");
	//	}
	//	disp_pos = 0;
	//}
	//else {
	//	out_char(p_tty->p_console, nr_tty + '0');
	//	out_char(p_tty->p_console, '#');
	//}

}

PRIVATE void exitFIND(CONSOLE* p_con){
	//先恢复原文的颜色
	onlyESC = 0;
	u8* p_vmem_start = (u8*)(V_MEM_BASE);
	for (int i = 0; i < start; i++) {
		//u8 ch = *(p_vmem_start + i * 2 + 1);
		if (*(p_vmem_start + i * 2 + 1) == RED_CHAR_COLOR) {
			*(p_vmem_start + i * 2 + 1) = DEFAULT_CHAR_COLOR;
		}
		else if (*(p_vmem_start + i * 2 + 1) == BLUE_CHAR_RED_BG) {
			*(p_vmem_start + i * 2 + 1) = BLUE_CHAR_COLOR;
		}
		else if (*(p_vmem_start + i * 2 + 1) == PURPLE_CHAR_RED_BG) {
			*(p_vmem_start + i * 2 + 1) =PURPLE_CHAR_COLOR;
		}
	}

	//再删除查找的字符串（光标移动回去）
	u8* p_str_start = (u8*)(V_MEM_BASE + start * 2);
	for (int i = 0; i < len; i++) {
		*(p_str_start + i * 2) = ' ';
		*(p_str_start + i * 2 + 1) = DEFAULT_CHAR_COLOR;
		p_con->cursor--;
	}
	//set_cursor(p_con->cursor);
}
PRIVATE void setRedStr(u8* p_str_start) {
	for (int i = 0; i < len; i++) {
		if (*(p_str_start + 1) == DEFAULT_CHAR_COLOR) {
			*(p_str_start + 1) = RED_CHAR_COLOR;
		}
		else if (*(p_str_start + 1) == BLUE_CHAR_COLOR) {
			*(p_str_start + 1) = BLUE_CHAR_RED_BG;
		}
		else if (*(p_str_start + 1) == PURPLE_CHAR_COLOR) {
			*(p_str_start + 1) = PURPLE_CHAR_RED_BG;
		}
		p_str_start += 2;
	}
}

PRIVATE void findStrAndShow() {
	len = end - start;
	u8* p_vmem_start = (u8*)(V_MEM_BASE);
	u8* p_str_start = (u8*)(V_MEM_BASE + start * 2);
	for (int i = 0; i < start-len+1; i++) {
		int isSame = 1;
		
		for (int j = 0; j < len; j++) {
			//普通字符 TAB空格 都要char相同
			if (*(p_vmem_start + i * 2 + j * 2)!= *(p_str_start + j * 2)) {
				isSame = 0;
				break;
			}
			//TAB空格 还要字的颜色一样
			if (*(p_str_start + j * 2) == ' ') {
				if(*(p_vmem_start + i * 2 + j * 2 + 1)==BLUE_CHAR_COLOR && *(p_str_start + j * 2 + 1)==PURPLE_CHAR_RED_BG) {
					isSame = 0;
					break;
				}
				if (*(p_vmem_start + i * 2 + j * 2 + 1) == PURPLE_CHAR_COLOR && *(p_str_start + j * 2 + 1) == BLUE_CHAR_RED_BG) {
					isSame = 0;
					break;
				}
			}
		}
		if (isSame == 1) setRedStr(p_vmem_start+i*2);
	}
}
/*======================================================================*
			   out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
	u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
	if (onlyESC == 1 && ch != 27) return;

	switch(ch) {
		case ' ':
			if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) {
				*p_vmem++ = ch;
				if(mode==INPUT)
					*p_vmem++ = PURPLE_CHAR_COLOR;
				else 
					*p_vmem++ = PURPLE_CHAR_RED_BG;
				p_con->cursor++;
			}
			break;
		case '\n':
			if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - SCREEN_WIDTH) {
				if (mode == INPUT) {
					u32 target_cursor = p_con->original_addr + SCREEN_WIDTH *
						((p_con->cursor - p_con->original_addr) /
							SCREEN_WIDTH + 1);
					while (p_con->cursor < target_cursor) {
						p_con->cursor++;
						*p_vmem++ = ' ';
						*p_vmem++ = DEFAULT_CHAR_COLOR;
					}
				}
				else if (mode == FIND) {
					onlyESC = 1;
					end = p_con->cursor;
					findStrAndShow();
					
				}
				
			}
			break;
		case '\b':
			if (p_con->cursor > p_con->original_addr) {
				if (mode == INPUT) {
					if (*(p_vmem - 2) == ' ' && *(p_vmem - 1) == BLUE_CHAR_COLOR) {
						//delete TAB
						for (int i = 0; i < 4; i++) {
							p_con->cursor--;
							p_vmem--;
							*p_vmem= DEFAULT_CHAR_COLOR;
							p_vmem--;
							*p_vmem = ' ';
						}
					}else if(*(p_vmem-2)==' ' && *(p_vmem-1)==DEFAULT_CHAR_COLOR){
						//delete \n
						while (*(p_vmem - 2) == ' ' && *(p_vmem - 1) == DEFAULT_CHAR_COLOR) {
							p_con->cursor--;
							p_vmem-=2;
						}
					}
					else {//delete 普通字符，比如空格，A
						p_con->cursor--;
						*(p_vmem - 2) = ' ';
						*(p_vmem - 1) = DEFAULT_CHAR_COLOR;
					}
				}
				else if (mode == FIND) {
					if (*(p_vmem - 2) == ' ' && *(p_vmem - 1) == BLUE_CHAR_RED_BG) {
						//delete TAB
						for (int i = 0; i < 4; i++) {
							p_con->cursor--;
							p_vmem--;
							*p_vmem = DEFAULT_CHAR_COLOR;
							p_vmem--;
							*p_vmem = ' ';
						}
					}
					else {//delete 普通字符，比如空格，A
						p_con->cursor--;
						*(p_vmem - 2) = ' ';
						*(p_vmem - 1) = DEFAULT_CHAR_COLOR;
					}
				
				}
			}
			break;
		case 27:
			if (mode == INPUT) {
				start = p_con->cursor;
				mode = FIND;
			}
			else if (mode == FIND && onlyESC==1) {
				mode = INPUT;
				exitFIND(p_con);
			}
			break;
		case '\t':
			if (mode == INPUT) {
				for (int i = 0; i < 4; i++) {
					*p_vmem++ = ' ';
					*p_vmem++ = BLUE_CHAR_COLOR;
					p_con->cursor++;
				}
			}
			else if(mode==FIND){
				for (int i = 0; i < 4; i++) {
					*p_vmem++ = ' ';
					*p_vmem++ = BLUE_CHAR_RED_BG;
					p_con->cursor++;
				}
			}
			break;
		default:
			if (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) {
				*p_vmem++ = ch;
				if(mode==INPUT)
					*p_vmem++ = DEFAULT_CHAR_COLOR;
				else if(mode==FIND)
					*p_vmem++ = RED_CHAR_COLOR;
				p_con->cursor++;
			}
			break;
	}

	while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
		scroll_screen(p_con, SCR_DN);
	}
	flush(p_con);
}

/*======================================================================*
			   clearAll 20s清屏
 *======================================================================*/
PUBLIC void clearAll(CONSOLE* p_console) {
	u8* start = (u8*)(V_MEM_BASE);
	u8* p_vmem = (u8*)(V_MEM_BASE + p_console->cursor * 2);
	
	while (start < p_vmem) {
		p_vmem--;
		*p_vmem = DEFAULT_CHAR_COLOR;
		p_vmem--;
		*p_vmem = ' ';
	}
	/* 默认光标位置在最开始处 */
	p_console->cursor = p_console->original_addr;
	//p_console->current_start_addr = p_console->original_addr;
	set_cursor(p_console->cursor);
	//flush(p_console);
}

/*======================================================================*
                           flush
*======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
        set_cursor(p_con->cursor);
        set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
			    set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, CURSOR_H);
	out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, CURSOR_L);
	out_byte(CRTC_DATA_REG, position & 0xFF);
	enable_int();
}

/*======================================================================*
			  set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
	disable_int();
	out_byte(CRTC_ADDR_REG, START_ADDR_H);
	out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
	out_byte(CRTC_ADDR_REG, START_ADDR_L);
	out_byte(CRTC_DATA_REG, addr & 0xFF);
	enable_int();
}



/*======================================================================*
			   scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
	if (direction == SCR_UP) {
		if (p_con->current_start_addr > p_con->original_addr) {
			p_con->current_start_addr -= SCREEN_WIDTH;
		}
	}
	else if (direction == SCR_DN) {
		if (p_con->current_start_addr + SCREEN_SIZE <
		    p_con->original_addr + p_con->v_mem_limit) {
			p_con->current_start_addr += SCREEN_WIDTH;
		}
	}
	else{
	}

	set_video_start_addr(p_con->current_start_addr);
	set_cursor(p_con->cursor);
}

