
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
			      console.h
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
						    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifndef _ORANGES_CONSOLE_H_
#define _ORANGES_CONSOLE_H_


/* CONSOLE */
typedef struct s_console
{
	unsigned int	current_start_addr;	/* 当前显示到了什么位置	  */
	unsigned int	original_addr;		/* 当前控制台对应显存位置 */
	unsigned int	v_mem_limit;		/* 当前控制台占的显存大小 */
	unsigned int	cursor;			/* 当前光标位置 */
}CONSOLE;

#define SCR_UP	1	/* scroll forward */
#define SCR_DN	-1	/* scroll backward */

#define SCREEN_SIZE		(80 * 25)
#define SCREEN_WIDTH		80

#define DEFAULT_CHAR_COLOR	0x07//黑底白字 换行填充的空格
#define RED_CHAR_COLOR	0x04	//黑底红字 ESC_INPUT输入可显示字符
#define BLUE_CHAR_COLOR 0x09	//黑底蓝字 INPUT输入TAB
#define PURPLE_CHAR_COLOR 0x0D  //黑底紫字 INPUT输入空格
#define BLUE_CHAR_RED_BG 0xC9	//红底蓝字 ESC_INPUT输入TAB
#define PURPLE_CHAR_RED_BG 0xCD	//红底紫字 ESC_INPUT输入空格

//#define RED_BG_COLOR  0xCC //空格和TAB的红底红字^ ^			//ESC_INPUT输入空格TAB

#endif /* _ORANGES_CONSOLE_H_ */
