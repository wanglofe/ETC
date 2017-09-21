 /****************************************************************************
 * Filename:    csr-access.c   												 *  
 * Author  :                                					             * 
 * Created :    															 *
 * Modified:    14/09/2017   Completely rewrite csr_access.c  by Kevin       * 
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <stdint.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <assert.h>
#include <string.h>
#include <sys/file.h>
#include <time.h>
#include <stdlib.h>
#include <stdarg.h>
#include <asm/errno.h>

#include "libipmi_session.h"
#include "libipmi_struct.h"
#include "coreTypes.h"
#include "libi2c.h"
#include "Types.h"
#include "Message.h"
#include "PDKHooks.h"
#include "IPMIConf.h"
#include "featuredef.h"
#include "libinspur.h"

#define STR2ULL(str) strtoull((str), NULL, 0)

static void show_usage(void);
static int do_operation(int argc, char **argv);

static int _cpu_read(int argc, char **argv); 
static int _cpu_write(int argc, char **argv);
static int _cpu_raw_read(int argc, char **argv);
static int _cpu_cpld_read(int argc, char **argv);
static int _cpu_cpld_write(int argc, char **argv); 
static int _nc_read(int argc, char **argv);
static int _nc_write(int argc, char **argv);
static int _nc_cpld_read(int argc, char **argv);
static int _nc_cpld_write(int argc, char **argv);
static int _io_cpld_read(int argc, char **argv); 
static int _io_cpld_write(int argc, char **argv);
static int _mc(int argc, char **argv);
static int _mc_pod_read(int argc, char **argv); 
static int _mc_pod_write(int argc, char **argv);
static int _memled_turnon(int argc, char **argv);
static int _memled_turnoff(int argc, char **argv);

static char *operationList[] = 
{
	"--cpu-read", 		
	"--cpu-write",			
	"--cpu-raw-read",	
	"--cpu-cpld-read", 	
	"--cpu-cpld-write",  
	"--nc-read",           
	"--nc-write",         
	"--nc-cpld-read",      
	"--nc-cpld-write",    
	"--io-cpld-read",     
	"--io-cpld-write",   
	"--mc",               
	"--mc-pod-read",       
	"--mc-pod-write",      
	"--memled-turnon",    
	"--memled-turnoff",
	NULL
};

static int (*handlerList[])(int, char **) = 
{
	_cpu_read,
	_cpu_write,
	_cpu_raw_read,
	_cpu_cpld_read,
	_cpu_cpld_write,
	_nc_read,
	_nc_write,
	_nc_cpld_read,
	_nc_cpld_write,
	_io_cpld_read,
	_io_cpld_write,
	_mc,
	_mc_pod_read,
	_mc_pod_write,
	_memled_turnon,
	_memled_turnoff
};

static void show_usage(void)
{
	 printf("\nusage:\n");
	 printf("csr_access  <operation>\t\t<arguments>\n");
	 printf("-------------------------------------------------------------------------------------------------\n");
	 printf("csr_access --cpu-read\t\t[index] [bus] [dev] [func] [reg] [length]\n");
	 printf("csr_access --cpu-write\t\t[index] [bus] [dev] [func] [reg] [length] [value]\n");
	 printf("csr_access --cpu-raw-read\t[index] [bus] [dev] [func] [reg] [length]\n");
	 printf("csr_access --cpu-cpld-read\t[index] [port]\n");
	 printf("csr_access --cpu-cpld-write\t[index] [port] [value]\n");
	 printf("csr_access --nc-read\t\t[index] [reg]\n");
	 printf("csr_access --nc-write\t\t[index] [reg] [value]\n");
	 printf("csr_access --nc-cpld-read\t[index] [port]\n");
 	 printf("csr_access --nc-cpld-write\t[index] [port] [value]\n");
 	 printf("csr_access --io-cpld-read\t[index] [port]\n");
 	 printf("csr_access --io-cpld-write\t[index] [port] [value]\n");
	 printf("csr_access --mc\t\t\t[reg]   [rom_file]\n");
	 printf("csr_access --mc-pod-read\t[index] [port] [tx/rx] [reg]\n");
	 printf("csr_access --mc-pod-write\t[index] [port] [tx/rx] [reg] [value]\n");
	 printf("csr_access --memled-turnon\t[cpuBoardindex] [cpuindex] [JCIndex] [channelIndex] [DimmIndex]\n");
	 printf("csr_access --memled-turnoff\t[cpuBoardindex] [cpuindex] [JCIndex] [channelIndex] [DimmIndex]\n");
	 printf("-------------------------------------------------------------------------------------------------\n");
}

int main(int argc, char **argv)
{
	int ret;
	
	if(argc < 2)
	{
		show_usage();
		return 1;
	}
	
	ret = do_operation( argc, argv );
	if( ret == 1 )
		show_usage();

	return 0;
}

/*
 * success 0, failed 1;
 */
static int do_operation(int argc, char **argv)
{
	int i = 0, lock_smbus = -1;
	
	while(operationList[i] != NULL)
	{
		if(strcmp(operationList[i], argv[1]) == 0)
		{	
			GET_SMBUS_LOCK(lock_smbus);
			wait_sem_lock(lock_smbus, BMC_MC_LOCK);
			
			//match, if operate failed return 1
			if( handlerList[i](argc, argv) )
			{   
				printf("Error arguments ! \n");
				return 1;
			}
			
			release_sem_lock(lock_smbus, BMC_MC_LOCK);
			
			break;
		}
		i++;
	}
	
	//mismatch
	if(operationList[i] == NULL)
	{	
		printf("Error operation ! \n");
		return 1;
	}
	
	return 0;
}

static int _cpu_read(int argc, char **argv)
{
	int ret;
	uint64 readdata = 0x0;
	
	if(argc != 8)
		return 1;
	
	ret = cpu_read_reg_cmd((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), (u8)STR2ULL(argv[4]),
			               (u8)STR2ULL(argv[5]), (u16)STR2ULL(argv[6]), (u8)STR2ULL(argv[7]), (u32 *)&readdata);
	if (ret == 0)
		printf("Success read CPU data: 0x%x \n", (u32)readdata);
	else
		printf("Failed read CPU data ! \n");
	
	return 0;
}

static int _cpu_write(int argc, char **argv)
{
	int ret;

	if(argc != 9)
		return 1;
	
	ret = cpu_write_reg_cmd((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), (u8)STR2ULL(argv[4]),
			                (u8)STR2ULL(argv[5]), (u16)STR2ULL(argv[6]), (u8)STR2ULL(argv[7]), (u32)STR2ULL(argv[8]));
	if (ret == 0)
		printf("Success write CPU data: 0x%x \n", (u32)STR2ULL(argv[8]));
	else
		printf("Failed write CPU data ! \n");
	
	return 0;
}

static int _cpu_raw_read(int argc, char **argv)
{
	int ret;
	uint64 readdata = 0x0;
	
	if(argc != 8)
		return 1;
	
	ret = cpu_read_reg_cmd_raw_package((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), (u8)STR2ULL(argv[4]),
			               	   	   	   (u8)STR2ULL(argv[5]), (u16)STR2ULL(argv[6]), (u8)STR2ULL(argv[7]), (u32 *)&readdata);
	if (ret == 0)
		printf("Success read CPU raw data: 0x%x \n", (u32)readdata);
	else
		printf("Failed read CPU raw data ! \n");
	
	return 0;
}

static int _cpu_cpld_read(int argc, char **argv)
{
	int ret;
	uint64 readdata = 0x0;
	
	if(argc != 4)
		return 1;
	
	ret = cpu_cpld_read((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), (u8 *)&readdata);
	
	if (ret == 0)
		printf("Success read CPU CPLD data: 0x%x \n", (u32)readdata);
	else
		printf("Failed read CPU CPLD data ! \n");
    
	return 0;
}

static int _cpu_cpld_write(int argc, char **argv)
{
	int ret;
	
	if(argc != 5)
		return 1;
	
	ret = cpu_cpld_write((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), (u8)STR2ULL(argv[4]));
	
	if (ret == 0)
		printf("Success write CPU CPLD data: 0x%x \n", (u8)STR2ULL(argv[4]));
	else
		printf("Failed read CPU CPLD data ! \n");
    
	return 0;	
}

static int _nc_read(int argc, char **argv)
{
	int ret;
	uint64 readdata = 0x0;
	
	if(argc != 4)
		return 1;
	
	ret = nc_read_reg((u8)STR2ULL(argv[2]), (u16)STR2ULL(argv[3]),&readdata);
	
	if (ret == 0)
		printf("Success read NC data: 0x%llx \n", readdata);
	else
		printf("Failed read NC data ! \n");
    
	return 0;
}

static int _nc_write(int argc, char **argv)
{
	int ret;
	
	if(argc != 5)
		return 1;
	
	ret = nc_write_reg((u8)STR2ULL(argv[2]), (u16)STR2ULL(argv[3]), (uint64)STR2ULL(argv[4]));
	
	if (ret == 0)
		printf("Success write NC data: 0x%llx \n", (uint64)STR2ULL(argv[4]));
	else
		printf("Failed write NC data ! \n");
    
	return 0;	
}

static int _nc_cpld_read(int argc, char **argv)
{
	int ret;
	uint64 readdata = 0x0;
	
	if(argc != 4)
		return 1;
	
	ret = nc_cpld_read((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]),(u8 *)&readdata);
	
	if (ret == 0)
		printf("Success read NC CPLD data: 0x%x \n", (u8)readdata);
	else
		printf("Failed read NC CPLD data ! \n");
    
	return 0;
}

static int _nc_cpld_write(int argc, char **argv)
{
	int ret;
	
	if(argc != 5)
		return 1;
	
	ret = nc_cpld_write((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), (u8)STR2ULL(argv[4]));
	
	if (ret == 0)
		printf("Success write NC CPLD data: 0x%x \n", (u8)STR2ULL(argv[4]));
	else
		printf("Failed read NC CPLD data ! \n");
    
	return 0;	
}

static int _io_cpld_read(int argc, char **argv)
{
	int ret;
	uint64 readdata = 0x0;
	
	if(argc != 4)
		return 1;
	
	ret = ioRiser_cpld_read((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]),(u8 *)&readdata);
	
	if (ret == 0)
		printf("Success read IO CPLD data: 0x%x \n", (u8)readdata);
	else
		printf("Failed read IO CPLD data ! \n");
    
	return 0;
}

static int _io_cpld_write(int argc, char **argv)
{
	int ret;
	
	if(argc != 5)
		return 1;
	
	ret = ioRiser_cpld_write((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), (u8)STR2ULL(argv[4]));
	
	if (ret == 0)
		printf("Success write IO CPLD data: 0x%x \n", (u8)STR2ULL(argv[4]));
	else
		printf("Failed read IO CPLD data ! \n");
    
	return 0;		
}

static int _mc(int argc, char **argv)
{
	int i;
	
	if(argc != 4)
		return 1;
	
	for(i=0;i<2;i++)
	{
		spico_mc_upload_test(i,(u16)STR2ULL(argv[2]), argv[3]);
	}
	//printf("Success spico_mc_upload_test ! \n");
	
	return 0;
}

static int _mc_pod_read(int argc, char **argv)
{
	int ret;
	u8 rxtxFlag;
	uint64 readdata = 0x0;
	
	if(argc != 6)
		return 1;
	
	(strcmp(argv[4],"rx") == 0) ? (rxtxFlag = 1) :(rxtxFlag = 0);
	
	ret = mc_pod_read_reg((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), rxtxFlag, (u8)STR2ULL(argv[5]),(u8 *)&readdata);
	
	if (ret == 0)
		printf("Success read MC POD data: 0x%x \n", (u8)readdata);
	else
		printf("Failed read MC POD data ! \n");
    
	return 0;
}

static int _mc_pod_write(int argc, char **argv)
{
	int ret;
	u8 rxtxFlag;
	
	if(argc != 7)
		return 1;
	
	(strcmp(argv[4],"rx") == 0) ? (rxtxFlag = 1) :(rxtxFlag = 0);
	
	ret = mc_pod_write_reg((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), rxtxFlag, (u8)STR2ULL(argv[5]),(u8)STR2ULL(argv[6]));
	
	if (ret == 0)
		printf("Success write MC POD data: 0x%x \n", (u8)STR2ULL(argv[6]));
	else
		printf("Failed write MC POD data ! \n");
    
	return 0;
}

static int _memled_turnon(int argc, char **argv)
{
	if(argc != 7)
		return 1;
	
	mem_led_turn_OnOff((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), (u8)STR2ULL(argv[4]), 
				       (u8)STR2ULL(argv[5]), (u8)STR2ULL(argv[6]), 1);
	
	//printf("Success memled turn on ! \n");
	
	return 0;
}

static int _memled_turnoff(int argc, char **argv)
{
	if(argc != 7)
		return 1;
	
	mem_led_turn_OnOff((u8)STR2ULL(argv[2]), (u8)STR2ULL(argv[3]), (u8)STR2ULL(argv[4]), 
				       (u8)STR2ULL(argv[5]), (u8)STR2ULL(argv[6]), 0);
	
	//printf("Success memled turn off ! \n");	
	
	return 0;
}
