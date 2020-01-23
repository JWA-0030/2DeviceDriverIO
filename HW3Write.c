/**
 * @file   HW3Write.c
 * @author Jesse Alsing
 * @date   April 1 2018
 * @version 0.1
 * Maps to /dev/HW3Write
 */

#include <linux/module.h>     	// Core header for loading LKMs into the kernel
#include <linux/kernel.h>     	// Contains types, macros, functions for the kernel
#include <linux/init.h>       	// Macros used to mark up functions e.g. __init __exit
#include <linux/device.h>      	// Header to support the kernel Driver Model
#include <linux/fs.h>         	// Header for the Linux file system support
#include <linux/uaccess.h>     	// Required for the copy to user function
#include <linux/mutex.h>		// Mutex lock support in Linux

#define SUCCESS 0
#define  DEVICE_NAME "HW3Write"	///< The device will appear at /dev/homework2char using this value
#define  CLASS_NAME  "HW3Write"		///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Jesse Alsing");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("Linux Character Driver");  ///< The description -- see modinfo
MODULE_VERSION("0.1");            ///< A version number to inform users

static int	  majorNumber;
static int    numberOpens = 0;              ///< Counts the number of times the device is opened
static struct class*  homework3WriteClass  = NULL; ///< The device-driver class struct pointer
static struct device* homework3WriteDevice = NULL; ///< The device-driver device struct pointer

extern char 	*message;
extern char   	*message_Ptr;
extern short 	size_of_message;  

static DEFINE_MUTEX(HW3Write_mutex);	//Mutex is created for use

// The prototype functions for the character driver -- must come before the struct definition
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
//static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

/** @brief Devices are represented as file structure in the kernel. The file_operations structure from
 *  /linux/fs.h lists the callback functions that you wish to associated with your file operations
 *  using a C99 syntax structure. char devices usually implement open, read, write and release calls
 */
static struct file_operations fops =
{
	.open = dev_open,
	//.read = dev_read,
	.write = dev_write,
	.release = dev_release,
};

/** @brief The LKM initialization function
 *  The static keyword restricts the visibility of the function to within this C file. The __init
 *  macro means that for a built-in driver (not a LKM) the function is only used at initialization
 *  time and that it can be discarded and its memory freed up after that point.
 *  @return returns 0 if successful
 */
static int __init HW3Write_init(void){
	
   printk(KERN_INFO "HW3Write: Initializing the Homework 3 write LKM\n");

   // Try to dynamically allocate a major number for the device -- more difficult but worth it
   majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNumber<0){
      printk(KERN_ALERT "Homework 3 write failed to register a major number\n");
      return majorNumber;
   }
   printk(KERN_INFO "Homework 3 write: registered correctly with major number %d\n", majorNumber);

   // Register the device class
   homework3WriteClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(homework3WriteClass)){                // Check for error and clean up if there is
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(homework3WriteClass);          // Correct way to return an error on a pointer
   }
   printk(KERN_INFO "Homework 3: device class registered correctly\n");

   // Register the device driver
   homework3WriteDevice = device_create(homework3WriteClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
   if (IS_ERR(homework3WriteDevice)){               // Clean up if there is an error
      class_destroy(homework3WriteClass);           // Repeated code but the alternative is goto statements
      unregister_chrdev(majorNumber, DEVICE_NAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(homework3WriteDevice);
   }
   
	//initiate the mutex for this class
	mutex_init(&HW3Write_mutex);
   printk(KERN_INFO "Homework 3: device class created correctly\n"); // Made it! device was initialized
   return SUCCESS;
}

/** @brief The LKM cleanup function
 *  Similar to the initialization function, it is static. The __exit macro notifies that if this
 *  code is used for a built-in driver (not a LKM) that this function is not required.
 */
static void __exit HW3Write_exit(void){
	//destroy the mutex on exit
	mutex_destroy(&HW3Write_mutex);
	
   device_destroy(homework3WriteClass, MKDEV(majorNumber, 0));   // remove the device
   class_unregister(homework3WriteClass);                        // unregister the device class
   class_destroy(homework3WriteClass);                           // remove the device class
   unregister_chrdev(majorNumber, DEVICE_NAME);  
   // unregister the major number
   printk(KERN_INFO "Homework 3 write: Goodbye from the LKM!\n");
}

// Report using printk each time its character device is opened.
/** @brief The device open function that is called each time the device is opened
 *  This will only increment the numberOpens counter in this case.
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_open(struct inode *inode, struct file *file){
	
	//static int counter = 0;
	
	//check if a lock is already implemented. if it is return.
	if(!mutex_trylock(&HW3Write_mutex)){
		printk(KERN_ALERT "homework 3 write: Device in use by another process");
	return -EBUSY;
	}
   numberOpens++;
   //sprintf(message, "I already told you %d times Hello world!\n", counter++);
   message_Ptr = message;
   try_module_get(THIS_MODULE);

   printk(KERN_INFO "Homework 3 write: Device has been opened %d time(s)\n", numberOpens);
   return SUCCESS;
}

/** @brief This function is called whenever the device is being written to from user space i.e.
 *  data is sent to the device from the user. The data is copied to the message[] array in this
 *  LKM using the sprintf() function along with the length of the string.
 *  @param filep A pointer to a file object
 *  @param buffer The buffer to that contains the string to write to the device
 *  @param len The length of the array of data that is being passed in the const char buffer
 *  @param offset The offset if required
 */
 //add extern into the function header
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
	
	if(strlen(buffer) + len > 256){
		
		strncpy(message, &buffer[0], 256);
		size_of_message = 256;
		printk(KERN_INFO "Homework 3 Write: Way too long");
	}
	else{
		sprintf(message, "(%zu letters)", len);  // appending received string with its length
		size_of_message = strlen(message); 

		// store the length of the stored message
		printk(KERN_INFO "Homework 3 write: Received %zu characters from the user\n", len);
	}
		
	return len;
}

// Report using printk each time its character device is closed.
/** @brief The device release function that is called whenever the device is closed/released by
 *  the userspace program
 *  @param inodep A pointer to an inode object (defined in linux/fs.h)
 *  @param filep A pointer to a file object (defined in linux/fs.h)
 */
static int dev_release(struct inode *inode, struct file *file){
	
	//release the lock 
	mutex_unlock(&HW3Write_mutex);
	
	numberOpens--;
	module_put(THIS_MODULE);
   
	printk(KERN_INFO "Homework 3 write: Device successfully closed\n");
	return 0;
}

/** @brief A module must use the module_init() module_exit() macros from linux/init.h, which
 *  identify the initialization function at insertion time and the cleanup function (as
 *  listed above)
 */
module_init(HW3Write_init);
module_exit(HW3Write_exit);
