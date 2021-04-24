/*   MODULE: keypad.c ------------------------------------------------- 
 * 
 *   Source file for building a shared library to be used by JNI for
 *   communicationg with the LKM (device driver) for the LED and
 *   keypad.
 ----------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>               //Communication facility between kernel and user space
#include <signal.h>
#include <stdbool.h>
#include "keypad.h"
	
								

//Define approrpiate logging function protoptye
//logf will be the alias for both Android App and normal Linux builds
#ifdef ANDROID_APP
	#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_DEBUG, "** NATIVE C++ **: ", __VA_ARGS__))
#else
    #define LOGI(...) printf(__VA_ARGS__)
#endif 


 
//IOCTL defines (for communication to kernel:
#define REG_CURRENT_TASK _IOW('a','a',int32_t)  //We use this to give a user application registration request to the kernel
#define IOCTL_WRITE_ON _IOW('a','b',int32_t)
#define IOCTL_WRITE_OFF _IOW('a','c',int32_t)
#define SIGMJM  44    //Signal value to identity user app
#define PW_NUM_CHARS 4   //Number of characters for the password
 

//Atrributes for password input from hardware
int pwCnt = 0;
char pw[PW_NUM_CHARS] = {'A','#', 'C','D'}; 
char input[PW_NUM_CHARS];
static int pw_valid = 1;

  
//File pointer for file commands  
int fd;
//Variable for ioctl() calls  
int32_t number;

//Flag for logging:
bool logging = true;
  
  
void keypad_hello(void)
{
	int num = 5;
	char *str = "Hello from keypad!";
	
	LOGI("Integer number: %d, String %s\n", num, str);
	 
		

}//keypad_hello

char* jni_test(void)
{
	//Test to check we can interface with Java space
	return "This is an updated jni test on 5 April from keypad!\n";
	 	

}//jni_test



//Callback function for processing signal from kernel
//NB: This handler is working for the CTL-C keyboard signal
void ctrl_c_handler(int n, siginfo_t *info,void *unused)
{
      if(n == SIGINT) //Check it is the right user ID
      {
         LOGI("\nWe have received the CTRL-C signal!\n");
      }   
      done = 1; //This will stop program
}


//Callback function for processing signal from kernel
void sig_event_handler(int n, siginfo_t *info,void *unused)
{
            
      char buf;
      //LOGI("User callback function called\n");
      
      if(n == SIGMJM)  //Check we have the right user id:
      {
         check = info->si_int;
         //LOGI("Received signal from kernel: Value = %d\n",check);
         buf = (char) check;
                 
          //LOGI("Received signal from kernel: Char Value = %c\n",buf);
           LOGI("%c\n",buf);
         #ifdef ANDROID_APP
		   char str[2];
           str[0] = buf;
           str[1] = '\0';
		   sendToJava(str);
		 #endif
				  
         
         if(pwCnt < PW_NUM_CHARS )
         { 
            input[pwCnt] = buf;
            pwCnt++;
         }
         if(pwCnt == PW_NUM_CHARS)  //Last char entered
         {
               //Check password:
               for(int i=0;i < PW_NUM_CHARS;i++)
               {
                     if(input[i] != pw[i])
                     {
                        #ifdef ANDROID_APP
						  char *str = "\nINVALID PASSWORD - PLEASE TRY AGAIN\n";
						  sendToJava(str);	
						#endif
                        LOGI("\nINVALID PASSWORD - PLEASE TRY AGAIN\n");
                        pw_valid = 0;
                        break;
                     }   
               }   
               if(pw_valid)
               {
                  #ifdef ANDROID_APP
                    char *str = "\nVALID PASSWORD!\n";
				    sendToJava(str);
				  #endif
				  LOGI("\nVALID PASSWORD!\n");
               }
               
               //Reset flags for next entries:
               pwCnt = 0;
               pw_valid = 1;
                  
               
         }      
      } 
      //done = 1;
      
}   


//Open the device driver
int init_dev(void)
{
	
	LOGI("Opening the keypad device driver\n");
	
	int32_t  number;
   struct sigaction act, ctrl;
   
   //Install CTRL-C Interrupt handler to clean up at exit:
   //sigemptyset (&ctrl.sa_mask);
   ctrl.sa_flags = (SA_SIGINFO | SA_RESETHAND);
   ctrl.sa_sigaction = ctrl_c_handler;
   if( sigaction(SIGINT,&ctrl,NULL) == -1)
   {
      LOGI("Problem setting up signal handler for SIGINTn");
   }   
   else
   {
      LOGI("Signal handler for SIGINT set up OK!\n");
   }
   
   
   //Set up signal handler:
   //sigemptyset (&act.sa_mask);
   //act.sa_flags = (SA_SIGINFO | SA_RESTART);
   act.sa_flags = SA_SIGINFO;
   act.sa_sigaction = sig_event_handler;
   if( sigaction(SIGMJM,&act,NULL) == -1)
   {
      LOGI("Problem setting up signal handler for SIGMJM = %d\n",SIGMJM);
   }   
   else
   {
      LOGI("Signal handler for SIGMJM = %d  set up OK!\n" ,SIGMJM);
   } 
   
   
		
   //Open driver:
   fd = open("/dev/rpichar_keypad", O_RDWR);             // Open the device with read/write access
   if (fd < 0)
   {
		LOGI("Failed to open the device driver...");
		LOGI("The error number: %d",errno);
		LOGI("Error description: %s",strerror(errno));
				
		return errno;
   }
   LOGI("Opened device driver OK");
   	
	//Register this application with the kernel:
   if( ioctl(fd, REG_CURRENT_TASK,(int32_t*) &number) < 0)
   {
      LOGI("User registration with kernel module has failed\n");
      close(fd);
      //exit(1);
      return 2;
   }   
  
   LOGI("User registration with kernel module SUCCESSFUL!\n");	
   return 0;
    	
	
}//init_dev
	
//Close the device driver	
void close_dev(void)
{
    LOGI("Closing the keypad device driver\n");
	close(fd);
	
}//close_dev
	
	
//Toggle the stae of the on-board LED	
void toggle(void)
{
	/*
	const char *wbuf = "ON";
		
	printf("Toggling the on board LED!\n");
	write(fd, wbuf,sizeof(wbuf));
	*/
	
	blinkLED(fd); 
	
	
	
	
}//toggle	


void switchMode(int mode)
{
	if(mode == 0)
	{
		//Switch off LED
		const char *wbuf = "OFF";
		
		LOGI("Switching LED Off!\n");
		write(fd, wbuf,sizeof(wbuf));
	}
	else if(mode == 1)
	{
		//Switch on LED
		const char *wbuf = "ON";
		
	    LOGI("Switching LED On!\n");
		write(fd, wbuf,sizeof(wbuf));
	}
		
}	

void ioctl_switchMode(int mode)
{
	//Alternative method to using write commands to device
	//(to accommodate Android set up)
	if(mode == 0)
	{
		//Send write command:
		if( ioctl(fd, IOCTL_WRITE_OFF,(int32_t*) &number) < 0)
		{
		  LOGI("ioctl Write command has failed\n");
		  //close(fd);
		  //exit(1);
		}   
		LOGI("ioctl Write command SUCCESSFUL!\n");
	    
	}
	if(mode == 1)
	{
		//Send write command:
		if( ioctl(fd, IOCTL_WRITE_ON,(int32_t*) &number) < 0)
		{
		  LOGI("ioctl Write command has failed\n");
		  //close(fd);
		  //exit(1);
		}   
		LOGI("ioctl Write command SUCCESSFUL!\n");
	}	
		
}	


void blinkLED(int fd)
{
   int numblinks = 10;
   int cnt = 0;
   for(int i=0;i<numblinks;i++)
   //while(1)
   {
      cnt++;
      
	  LOGI("Toggling light switch count %d\n", cnt);
      sleep(1);
      const char *wbuf = "TOG";
      write(fd, wbuf,sizeof(wbuf));
      
   }
}   


//---------------------------------------------------------------------
//DEFINE METHODS FOR JAVA CALLBACK HERE (If Android App realisation):
#ifdef ANDROID_APP
void init_callbacks(char *f_name, char *f_sig)
{
	//strcpy(callback_name,f_name);
	callback_name = f_name;
	callback_sig = f_sig;
	LOGI("Set callback strings OK!");
	LOGI("Callback name: %s",callback_name);
	LOGI("Callback signature: %s",callback_sig);
}
	
void sendToJava(char *c)
{
	
	//LOGI("JNIEnv pointer 0x%p",glo_env);
	//LOGI("JVM pointer 0x%p",JVM);
	//LOGI("Global class 0x%p",&glo_clazz);
	//LOGI("Global object 0x%p",&glo_obj);
    
	    
    jstring js = (*glo_env)->NewStringUTF(glo_env,c);
    jclass clazz = (*glo_env)->GetObjectClass(glo_env,glo_obj);
	jmethodID mid_cb = (*glo_env)->GetMethodID(glo_env,clazz,callback_name, callback_sig);
    (*glo_env)->CallVoidMethod(glo_env, glo_obj,mid_cb,js); //OK with this
      
    
	LOGI("Data sent to Java!");
	
	
}	

#endif	
//---------------------------------------------------------------------
