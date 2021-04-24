/*   MODULE HEADER FILE: keypad.c --------------------------------------- 
 * 
 *   Source file for building a shared library to be used by JNI for
 *   communicationg with the LKM (device driver) for the LED and
 *   keypad.
 ----------------------------------------------------------------------*/ 
#ifndef KEYPAD_H
#define KEYPAD_H


#define ANDROID_APP      //Add this define if we are running C Code as part of Android App 
                         //to facilitate adb logging          
#ifdef ANDROID_APP
 #include <android/log.h>
 #include <jni.h>
#endif
	

void keypad_hello(void);
char* jni_test(void);
int init_dev(void);
void close_dev(void);
void toggle(void);
void switchMode(int mode);
void ioctl_switchMode(int mode);

void blinkLED(int fd);

#ifdef ANDROID_APP
	void init_callbacks(char *f_name, char *f_sig);
	void sendToJava(char *c);
#endif


//Attributes:
int done = 0;
int check = 0;


#ifdef ANDROID_APP
	JNIEnv *glo_env;
	JavaVM *JVM;
	jclass glo_clazz;
	jobject glo_obj;
	char *callback_name; 
	char *callback_sig;
#endif

#endif
