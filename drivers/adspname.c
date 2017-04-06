/* #############################################################################
 *****************************************************************************
 Copyright (c) 2005, Advantech eAutomation Division
 Advantech Co., Ltd.
 Linux DIO driver
 THIS IS AN UNPUBLISHED WORK CONTAINING CONFIDENTIAL AND PROPRIETARY
 INFORMATION WHICH IS THE PROPERTY OF ADVANTECH AUTOMATION CORP.

 ANY DISCLOSURE, USE, OR REPRODUCTION, WITHOUT WRITTEN AUTHORIZATION FROM
 ADVANTECH AUTOMATION CORP., IS STRICTLY PROHIBITED.
 *****************************************************************************
#############################################################################

File: 	    adspname.c
Version:         1.00 <01/16/2006>
Author:          Joshua Lan
Change log:      Version 1.00 <01/16/2006>
- Inivial version

Description:     This is a virtual driver to detect Advantech module.
Status: 	    works

Permission to use, copy, modify, and distribute this software and its
documentation for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.  This software is provided "as is" without express or
implied warranty.
----------------------------------------------------------------------------*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/version.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/ioctl.h>
#include <asm/io.h>
#include <linux/pci.h>
#include <linux/poll.h>
#include <linux/wait.h>
#include <linux/completion.h>
#include <linux/sched.h>
#include <linux/param.h>		
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/spinlock.h>

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) ((a)*65536+(b)*256+(c))
#endif

#define ADVANTECH_DIO_VER               "1.02"
#define ADVANTECH_DIO_DATE              "08/09/2016" 

#define ADSPNAME_MAGIC 			'p'
#define GETPNAME			_IO(ADSPNAME_MAGIC, 1)
#define CHKADVBOARD			_IO(ADSPNAME_MAGIC, 2)

//#define ADVANTECH_DEBUG
#ifdef ADVANTECH_DEBUG
#define DEBUGPRINT printk
#else
#define DEBUGPRINT(a, ...)
#endif

#define DEVICE_NODE_NAME "adspname"
#define DEVICE_CLASS_NAME "adspname"
#define DEVICE_FILE_NAME "adspname"
#define ADVSPNAME_MAJOR 0
#define DEBUG
#define PPC_DEVICE
/****************************************/
#define _ADVANTECH_BOARD_NAME_LENGTH        64
#define SEARCH_BOARD_NAME_LENGTH			65536
#define SEARCH_BOARD_NAME_ADDRESS			0x000F0000

#define AWARD_BIOS_NAME                     "Phoenix - AwardBIOS"
#define AWARD_BIOS_NAME_ADDRESS             0x000FE061
#define AWARD_BIOS_NAME_LENGTH              strlen(AWARD_BIOS_NAME)
#define AWARD_ADVANTECH_BOARD_ID_ADDRESS    0x000FEC80
#define AWARD_ADVANTECH_BOARD_ID_LENGTH     32
#define AWARD_ADVANTECH_BOARD_NAME_ADDRESS  0x000FE0C1
#define AWARD_ADVANTECH_BOARD_NAME_LENGTH   _ADVANTECH_BOARD_NAME_LENGTH

#define AMI_BIOS_NAME                       "AMIBIOS"
#define AMI_BIOS_NAME_ADDRESS               0x000FF400
#define AMI_BIOS_NAME_LENGTH                strlen(AMI_BIOS_NAME)
#define AMI_ADVANTECH_BOARD_ID_ADDRESS      0x000FE840
#define AMI_ADVANTECH_BOARD_ID_LENGTH       32
#define AMI_ADVANTECH_BOARD_NAME_LENGTH     _ADVANTECH_BOARD_NAME_LENGTH
/***************************************/

static unsigned char is_adv_dev[8] = "yes";
static unsigned char no_adv_dev[8] = "no";
static unsigned char board_id[_ADVANTECH_BOARD_NAME_LENGTH];
static bool check_result = false;

//static int iobase = 0x000FE0C1;

//static int io_range = 32;

struct adspname_cdev{
	struct cdev dev;
	spinlock_t adspname_spinlock;
};

struct class *my_class;
int adspname_major = ADVSPNAME_MAJOR;
static unsigned char * uc_ptaddr;
struct adspname_cdev * ads_cdev= NULL;

static int adspname_ioctl (
		struct file *file,
		unsigned int cmd,
		unsigned long arg )
{  
	/*
	   unsigned short sdata[ 4 ];
	   int idata[ 4 ];
	   long ldata[ 4 ];
	   int options;//, retval = -EINVAL;
	   */

	DEBUGPRINT("in ioctl()\n");

	switch ( cmd )
	{
	case GETPNAME:
#ifdef DEBUG
		printk(KERN_INFO "Board name is: %s\n", board_id);
#endif
		copy_to_user((void*)arg, board_id, sizeof(board_id));
		break;
	case CHKADVBOARD:
#ifdef DEBUG
		printk(KERN_INFO "Board name is: %s\n", board_id);
#endif
		if(check_result)
			copy_to_user((void*)arg, is_adv_dev, sizeof(is_adv_dev));
		else
			copy_to_user((void*)arg, no_adv_dev, sizeof(no_adv_dev));
		break;

	default:
		return -1;
	}
	return 0;
}
// *****************************************************************************
// Design Notes:  
// -----------------------------------------------------------------------------
static int adspname_open (
		struct inode *inode, 
		struct file *file )
{
	DEBUGPRINT("in open()\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_INC_USE_COUNT;
#endif
	struct adspname_cdev *dev;
	dev = container_of(inode->i_cdev,struct adspname_cdev,dev);
	file->private_data = dev;
	return 0;
}

// *****************************************************************************
// Design Notes:  
// -----------------------------------------------------------------------------
static int adspname_release (
		struct inode *inode, 
		struct file *file )
{
	DEBUGPRINT("in release\n");
#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,0)
	MOD_DEC_USE_COUNT;
#endif
	return 0;
}

// *****************************************************************************
// Design Notes:  
// -----------------------------------------------------------------------------
static struct file_operations adspname_fops = {
	.owner		=THIS_MODULE,
	.unlocked_ioctl = adspname_ioctl,
	.open		=adspname_open,
	.release	=adspname_release,
};

// *****************************************************************************
// Design Notes:  
// -----------------------------------------------------------------------------
void adspname_cleanup ( void )
{
	/*if ( unregister_chrdev( major, "adspname" ) !=0 )
	  {
	  DEBUGPRINT( DEVICE_NAME " unregister of device failed!\n");
	  }*/
	dev_t devno=MKDEV(adspname_major,0);
	device_destroy(my_class,devno);
	class_destroy(my_class);
	cdev_del(&ads_cdev->dev);
	kfree(ads_cdev);
	unregister_chrdev_region(devno,1);
	//unregister_chrdev( major, "adspname" );
	iounmap(( void* )uc_ptaddr );
	DEBUGPRINT("in cleanup\n");
}
/******************************************************************************/
static char * _MapIoSpace(unsigned int HWAddress, unsigned long Size)
{
	char *              ioPortBase = NULL;
	ioPortBase = ioremap_nocache(HWAddress, Size);
	return (ioPortBase);
}

static void _UnMapIoSpace(char * BaseAddress, unsigned long Size)
{
	iounmap(BaseAddress);
}

static bool _IsBiosMatched(unsigned int BiosAddress, char *pBiosName, int BiosNameLen)
{
	bool bMatched = false;
	char * pVMem;

	pVMem = _MapIoSpace(BiosAddress, BiosNameLen);
	if(!pVMem) return false;
	if(!strncmp((char *)pVMem, pBiosName, BiosNameLen))
		bMatched = true;
	_UnMapIoSpace(pVMem, BiosNameLen);

	return bMatched;
}

/********************************************************************************/
// *****************************************************************************
// Design Notes:  
//      init our Module
// -----------------------------------------------------------------------------
int adspname_init ( void )
{ 
	dev_t devno;
	int ret;
	int loopc = 0;
	int length = 0;

	if(adspname_major)
	{
		devno = MKDEV(adspname_major,0);
		ret = register_chrdev_region(devno,1,DEVICE_NODE_NAME);
		if(ret<0)
		{
			DEBUGPRINT("register fail!\r\n");
			goto exit0;
		}
	}
	else
	{
		ret = alloc_chrdev_region(&devno,0,1,DEVICE_NODE_NAME);
		if(ret<0)
		{
			DEBUGPRINT("register fail!\r\n");
			goto exit0;
		}
		adspname_major = MAJOR(devno);
	}

	ads_cdev = kmalloc(sizeof(struct adspname_cdev),GFP_KERNEL);
	if(!ads_cdev)
	{
		ret = -ENOMEM;
		goto exit1;
	}

	memset(ads_cdev,0,sizeof(struct adspname_cdev));
	spin_lock_init(&ads_cdev->adspname_spinlock);
	cdev_init(&ads_cdev->dev,&adspname_fops);
	ads_cdev->dev.owner = THIS_MODULE;
	ret = cdev_add(&ads_cdev->dev,devno,1);
	if(ret<0)
	{
		DEBUGPRINT("cdev add fail!\r\n");
		goto exit2;
	}

	my_class = class_create(THIS_MODULE,DEVICE_CLASS_NAME);
	if(IS_ERR(my_class))
	{
		DEBUGPRINT("class create fail!\r\n");
		goto exit3;
	}

	device_create(my_class,NULL,devno,"%s",DEVICE_FILE_NAME);

	//if((register_chrdev( 250, "adspname", &adspname_fops)) < 0)
	//{
	//	printk("register_chrdev failed!\n");
	//	return -ENODEV;
	//}

	// If Award BIOS
	if(_IsBiosMatched(AWARD_BIOS_NAME_ADDRESS, AWARD_BIOS_NAME, AWARD_BIOS_NAME_LENGTH))
	{
#ifdef DEBUG
		printk(KERN_INFO "=====================================================\n");
		printk(KERN_INFO "     Product Name Detecting driver V%s [%s]\n", 
				ADVANTECH_DIO_VER, ADVANTECH_DIO_DATE);
		printk(KERN_INFO "     Advantech eAutomation Division.\n");
		printk(KERN_INFO "=====================================================\n");
		printk(KERN_INFO "Award BIOS\n");
#endif
	}
	// If AMI BIOS
	else //if(_IsBiosMatched(AMI_BIOS_NAME_ADDRESS, AMI_BIOS_NAME, AMI_BIOS_NAME_LENGTH))
	{
#ifdef DEBUG
		printk(KERN_INFO "=====================================================\n");
		printk(KERN_INFO "     Product Name Detecting driver V%s [%s]\n", 
				ADVANTECH_DIO_VER, ADVANTECH_DIO_DATE);
		printk(KERN_INFO "     Advantech eAutomation Division.\n");
		printk(KERN_INFO "=====================================================\n");
		printk(KERN_INFO "AMI BIOS\n");
#endif
	}

	uc_ptaddr = ioremap_nocache(SEARCH_BOARD_NAME_ADDRESS, SEARCH_BOARD_NAME_LENGTH);
	check_result = false;
	for(loopc = 0; loopc < SEARCH_BOARD_NAME_LENGTH; loopc++)
	{
		if((uc_ptaddr[loopc]=='T' && uc_ptaddr[loopc+1]=='P' && uc_ptaddr[loopc+2]=='C')
				|| (uc_ptaddr[loopc]=='P' && uc_ptaddr[loopc+1]=='P' && uc_ptaddr[loopc+2]=='C')
				|| (uc_ptaddr[loopc]=='U' && uc_ptaddr[loopc+1]=='N' && uc_ptaddr[loopc+2]=='O')
				|| (uc_ptaddr[loopc]=='I' && uc_ptaddr[loopc+1]=='T' && uc_ptaddr[loopc+2]=='A')
				|| (uc_ptaddr[loopc]=='A' && uc_ptaddr[loopc+1]=='I' && uc_ptaddr[loopc+2]=='M' && uc_ptaddr[loopc+3]=='C')
				|| (uc_ptaddr[loopc]=='A' && uc_ptaddr[loopc+1]=='P' && uc_ptaddr[loopc+2]=='A' && uc_ptaddr[loopc+3]=='X')
				|| (uc_ptaddr[loopc]=='M' && uc_ptaddr[loopc+1]=='I' && uc_ptaddr[loopc+2]=='O'))
		{
			check_result = true;
			length = 2;
			while((uc_ptaddr[loopc + length] != ' ') && (length < _ADVANTECH_BOARD_NAME_LENGTH))
			{
				length += 1;
			}
			break;
		}
	}

	memset(board_id, 0, sizeof(board_id));

	if(check_result)
	{
		memmove(board_id, uc_ptaddr+loopc, length);
#ifdef DEBUG
		printk(KERN_INFO "loopc: %d\n", loopc);
		printk(KERN_INFO "Is: %s\n", board_id);
#endif
	}
	else
	{
#ifdef DEBUG
		printk(KERN_INFO "Is no advantech device!\n");
#endif
	}

	return 0;

exit3:
	cdev_del(&ads_cdev->dev);
exit2:
	kfree(ads_cdev);
exit1:
	unregister_chrdev_region(devno,1);
exit0:
	return -1;
}

module_init( adspname_init );
module_exit( adspname_cleanup );

MODULE_DESCRIPTION("Driver for ADVANTECH Devices Detecting");
MODULE_LICENSE("GPL");

