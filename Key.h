/*
 * Key.h
 * 
 * Interface for Key
 * 
 * Created by:
 *      K.C.Y
 * Date:
 *      2017/07
 */
#ifndef _KEY_H_
#define _KEY_H_

typedef void (*key_press_fn_t)(void);

void key_init(int key_pin, int long_press_time_ms, key_press_fn_t long_press_fn);

#endif

