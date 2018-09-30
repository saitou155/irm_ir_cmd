#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <libusb-1.0/libusb.h>
#include <json-c/json.h>

#define VENDOR_ID	0x04d8
#define PRODUCT_ID	0x000a

#define EP_IN_ADDR	0x83
#define EP_OUT_ADDR	0x03

#define CONTROL_REQUEST_TYPE_IN (LIBUSB_ENDPOINT_IN | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE)
#define CONTROL_REQUEST_TYPE_OUT (LIBUSB_ENDPOINT_OUT | LIBUSB_REQUEST_TYPE_CLASS | LIBUSB_RECIPIENT_INTERFACE)

//	cdc-acm.h
#define ACM_CTRL_DTR   0x01
#define ACM_CTRL_RTS   0x02

//	cdc.h
#define USB_CDC_REQ_SET_LINE_CODING		0x20
#define USB_CDC_REQ_GET_LINE_CODING		0x21
#define USB_CDC_REQ_SET_CONTROL_LINE_STATE	0x22
#define USB_CDC_REQ_SEND_BREAK			0x23

libusb_context *ctx = NULL; 
libusb_device_handle *devh = NULL;
void dump(const char* data);
void usage();

int irm_open()
{
	int ret;
	
	ret = libusb_init(&ctx);
	if(ret < 0)
	{
		fprintf(stderr, "ERROR:libusb initialize failed. %s\n",libusb_error_name(ret));
		return -1;
	}
	
	libusb_set_debug(NULL, 3);
	
	devh = libusb_open_device_with_vid_pid(NULL, VENDOR_ID, PRODUCT_ID);
	if(!devh)
	{
		fprintf(stderr, "ERROR:Can't Open irMagician. Device not found.n");
		return -1;
	}
	
	for (int if_num = 0; if_num < 2; if_num++) 
	{
		if (libusb_kernel_driver_active(devh, if_num)) 
		{
			libusb_detach_kernel_driver(devh, if_num);
		}
		ret = libusb_claim_interface(devh, if_num);
		if (ret < 0) 
		{
			fprintf(stderr, "ERROR:Can't claim interface. %s\n",libusb_error_name(ret));
			return -1;
		}
	}
	
	ret = libusb_control_transfer(devh, 
								  CONTROL_REQUEST_TYPE_OUT, USB_CDC_REQ_SET_CONTROL_LINE_STATE, 
								  ACM_CTRL_DTR | ACM_CTRL_RTS, 0, NULL, 0, 0);
    if (ret < 0) 
	{
		fprintf(stderr, "ERROR:Can't set control line. %s\n",libusb_error_name(ret));
			return -1;
    }

    /* cdc.h - struct usb_cdc_line_coding
     * 9600 = 0x2580 ~> 0x80, 0x25 in little endian
     */
    unsigned char encoding[] = { 0x80, 0x25, 0x00, 0x00, 0x00, 0x00, 0x08 };
    ret = libusb_control_transfer(devh, 
								  CONTROL_REQUEST_TYPE_OUT, USB_CDC_REQ_SET_LINE_CODING, 
								  0, 0, encoding, sizeof(encoding), 0);
    if (ret < 0) 
	{
		fprintf(stderr, "ERROR:Can't set baud rate. %s\n",libusb_error_name(ret));
			return -1;
    }
	
	return 0;
}

int irm_close()
{
	int ret = -1;
    if (devh)
	{
	    libusb_release_interface(devh, 0);
        libusb_close(devh);
		ret = 0;
	}
	if (ctx)
	{
    	libusb_exit(ctx);
	}
	
	return ret;
}

int irm_puts(unsigned char * data, int size)
{
    int length;
	int ret;
	
	ret = libusb_bulk_transfer(devh, EP_OUT_ADDR, data, size, &length, 200);
    if (ret < 0) 
	{
		fprintf(stderr, "ERROR:while sending char. %s\n",libusb_error_name(ret));
		return -1;
    }
	return length;
}

int irm_gets(unsigned char * data, int size)
{
    int length,tmp;
	unsigned int timeout;
    int ret;
	
	length = 0;
	memset(data,0,size);
	while(1)
	{
		if(length == 0)
		{
			timeout = 200;
		}
		else
		{
			timeout = 1;
		}
		ret = libusb_bulk_transfer(devh, EP_IN_ADDR, data + length, size - length, &tmp, timeout);
		if (ret == LIBUSB_ERROR_TIMEOUT) 
		{
			if(length == 0)
			{
				fprintf(stderr, "ERROR:receive timeout. length=%d\n",length);
				return -1;
			}
			else
			{
				break;
			}
		} 
		else if (ret < 0) 
		{
			fprintf(stderr, "ERROR:while receiveing char. %s\n",libusb_error_name(ret));
			return -1;
		}
		length += tmp;
	}

    return length;
}

int irm_tciflush()
{
	unsigned char data[64];
    int length;
	int ret;
	int result = 0;
	
	while(1)
	{
    	ret = libusb_bulk_transfer(devh, EP_IN_ADDR, data, sizeof(data), &length,200);
		if (ret < 0 || length == 0) 
		{
			break;
		}
		result = 1;
	}
	return result;
}

int irm_cmd(const char* cmd1,int verbose)
{
	int result = -1;
	int ret;
	char buf[256];
	
	ret = irm_puts((char*)cmd1, strlen(cmd1));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	if(verbose)
	{
		fprintf(stderr,"%s",cmd1);
	}

	result = 0;
EXIT_PATH:
	return result;
}

int irm_cmd_res2(const char* cmd1,const char* res1,char* data,int size,int verbose)
{
	int result = -1;
	int ret;
	char buf[256];
	
	ret = irm_puts((char*)cmd1, strlen(cmd1));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	if(verbose)
	{
		fprintf(stderr,"%s",cmd1);
	}

	sleep(1);
	
	ret = irm_gets(buf,sizeof(buf));
	if(ret < 0)
	{
		fprintf(stderr,"No response form IrMagician.\n");
		goto EXIT_PATH;
	}
	if(verbose)
	{
		int len = strlen(buf);
		if(buf[len-1] != '\n')
		{
			fprintf(stderr,"%s\n",buf);
		}
		else
		{
			fprintf(stderr,"%s",buf);
		}
	}
	if(strcmp(buf,res1))
	{
		fprintf(stderr,"No response form IrMagician.\n");
		goto EXIT_PATH;
	}
	
	sleep(3);
	
	ret = irm_gets(data,size);
	if(ret < 0)
	{
		fprintf(stderr,"No response form IrMagician.\n");
		goto EXIT_PATH;
	}
	if(verbose)
	{
		int len = strlen(data);
		if(data[len-1] != '\n')
		{
			fprintf(stderr,"%s\n",data);
		}
		else
		{
			fprintf(stderr,"%s",data);
		}
	}
	
	result = ret;
EXIT_PATH:
	return result;
}

int irm_cmd_res(const char* cmd1,char* data,int size,int verbose)
{
	int result = -1;
	int ret;
	
	ret = irm_puts((char*)cmd1, strlen(cmd1));
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	if(verbose)
	{
		fprintf(stderr,"%s",cmd1);
	}
	
	ret = irm_gets(data,size);
	if(ret < 0)
	{
		fprintf(stderr,"No response form IrMagician.\n");
		goto EXIT_PATH;
	}
	if(verbose)
	{
		int len = strlen(data);
		if(data[len-1] != '\n')
		{
			fprintf(stderr,"%s\n",data);
		}
		else
		{
			fprintf(stderr,"%s",data);
		}
	}
	result = ret;
EXIT_PATH:
	return result;
}

int irm_receive(int varbose)
{
	int result = -1;
	int ret;
	int len,recNo,pScale,mem;
	char cmd[32];
	char buf[32];
	unsigned char range[640];
	
	fprintf(stderr,"Capturing IR...\n");
	
	ret = irm_open();
	if(ret < 0)
	{
		goto EXIT_PATH;
	}

	irm_tciflush();
	
	ret = irm_cmd_res2("c\r\n","...",buf,sizeof(buf),varbose);
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	len = -1;
	ret = sscanf(buf," %d",&len);
	if(ret < 1 || len < 1)
	{
		if(!varbose)
		{
			fprintf(stderr," Time Out !\n");
		}
		goto EXIT_PATH;
	}
	
	ret = irm_cmd_res("I,1\r\n",buf,sizeof(buf),varbose);
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	recNo = -1;
	ret = sscanf(buf," %x",&recNo);
	if(ret < 1 || recNo < 1)
	{
		goto EXIT_PATH;
	}

	ret = irm_cmd_res("I,6\r\n",buf,sizeof(buf),varbose);
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	pScale = -1;
	ret = sscanf(buf," %d",&pScale);
	if(ret < 1 || pScale < 1)
	{
		goto EXIT_PATH;
	}
	
	for(int i = 0;i < recNo;i++)
	{
		int nPage = i / 64;
		int nOff  = i % 64;
		
		if(!nOff)
		{
			sprintf(cmd,"B,%d\r\n",nPage);
			ret = irm_cmd(cmd,varbose);
			if(ret < 0)
			{
				goto EXIT_PATH;
			}
		}
		sprintf(cmd,"D,%d\r\n",nOff);
		ret = irm_cmd_res(cmd,buf,sizeof(buf),varbose);
		if(ret < 0)
		{
			goto EXIT_PATH;
		}
		mem = -1;
		ret = sscanf(buf," %x",&mem);
		if(ret < 1 || mem < 1)
		{
			goto EXIT_PATH;
		}
		range[i] = (unsigned char)(mem & 0xff);
	}
	
	json_object *pJSON = json_object_new_object();
	json_object_object_add(pJSON, "format", json_object_new_string("raw"));
	json_object_object_add(pJSON, "freq", json_object_new_int(38));
	json_object *pARRAY = json_object_new_array();
	for(int i = 0;i < recNo;i++)
	{
		json_object_array_add(pARRAY,json_object_new_int(range[i]));
	}
	json_object_object_add(pJSON, "data", pARRAY);
	json_object_object_add(pJSON, "postscale", json_object_new_int(pScale));
	printf("%s\n",json_object_to_json_string(pJSON));
	json_object_put(pJSON);
	
	result = 0;
EXIT_PATH:
	irm_close();
	return result;
}

int irm_transfer(const char* json,int varbose)
{
	int result = -1;
	
	json_object *pJSON;
	json_object * pDATA;
	json_object * pSCALE;
	json_object * pITEM;
	char cmd[32];
	char buf[32];

	int ret;
	int recNo,pScale;
	unsigned char range[640];

	pJSON = json_tokener_parse(json);
	if(pJSON == NULL)
	{
		if(!varbose)
		{
			fprintf(stderr,"ERROR:Can't parse json.\n");
		}
		else
		{
			fprintf(stderr,"ERROR:Can't parse json.\"%s\"\n",json);
		}
		goto EXIT_PATH;
	}
	
	if(!json_object_object_get_ex(pJSON, "postscale",&pSCALE))
	{
		fprintf(stderr,"ERROR:not found postscale in json.\n");
		goto EXIT_PATH;
	}
	if(json_object_get_type(pSCALE) != json_type_int)
	{
		fprintf(stderr,"ERROR:postscale was not integer.\n");
		goto EXIT_PATH;
	}
	pScale = json_object_get_int(pSCALE);
		
	if(!json_object_object_get_ex(pJSON, "data",&pDATA))
	{
		fprintf(stderr,"ERROR:not found data in json.\n");
		goto EXIT_PATH;
	}
	if(json_object_get_type(pDATA) != json_type_array)
	{
		fprintf(stderr,"ERROR:data was not array.\n");
		goto EXIT_PATH;
	}
	recNo = json_object_array_length(pDATA);
	for(int i = 0;i < recNo;i++)
	{
		pITEM = json_object_array_get_idx(pDATA, i);
		if (json_object_get_type(pITEM) != json_type_int) 
		{
			fprintf(stderr,"ERROR:data[%d] was not integer.\n",i);
			goto EXIT_PATH;
		}
		range[i] = (char)(json_object_get_int(pITEM) & 0xff);
	}
	json_object_put(pJSON);
	
	fprintf(stderr,"Transfer IR...\n");

	ret = irm_open();
	if(ret < 0)
	{
		goto EXIT_PATH;
	}

	irm_tciflush();
	
	sprintf(cmd,"N,%d\r\n",recNo);
	ret = irm_cmd_res(cmd,buf,sizeof(buf),varbose);
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	if(strcmp(buf,"OK\r\n"))
	{
		goto EXIT_PATH;
	}

	sprintf(cmd,"K,%d\r\n",pScale);
	ret = irm_cmd_res(cmd,buf,sizeof(buf),varbose);
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	if(strcmp(buf,"OK\r\n"))
	{
		goto EXIT_PATH;
	}

	for(int i = 0;i < recNo;i++)
	{
		int nPage = i / 64;
		int nOff  = i % 64;
		
		if(!nOff)
		{
			sprintf(cmd,"B,%d\r\n",nPage);
			ret = irm_cmd(cmd,varbose);
			if(ret < 0)
			{
				goto EXIT_PATH;
			}
		}
		sprintf(cmd,"W,%d,%d\r\n",nOff,(int)range[i]);
		ret = irm_cmd(cmd,varbose);
		if(ret < 0)
		{
			goto EXIT_PATH;
		}
	}
	
	ret = irm_cmd_res("P\r\n",buf,sizeof(buf),varbose);
	if(ret < 0)
	{
		goto EXIT_PATH;
	}
	if(strcmp(buf,"..."))
	{
		goto EXIT_PATH;
	}

EXIT_PATH:
	irm_close();
	return result;
}

int main(int argc,char * argv[])
{
	int ret = -1;
	int varbose = 0;
	char cmd = '\0';
	char* json = NULL;
	
	int opt;
	while ((opt = getopt(argc, argv, "vrt:")) != -1) 
	{
        switch (opt) 
		{
            case 'r':
				cmd = opt;
                break;
            case 'v':
				varbose = 1;
                break;
            case 't':
				cmd = opt;
				json = optarg;
                break;
        }
    }

	if(cmd == '\0')
	{
		usage();
		return 0;
	}
	
	switch(cmd)
	{
		case 'r': 
			ret = irm_receive(varbose);
			break;
		case 't': 
			ret = irm_transfer(json,varbose);
			break;
	}
	
	return ret;
}

void usage() 
{
  fprintf(stderr, "usage: irm_ir_cmd <option>\n");
  fprintf(stderr, "  -r       \tReceive Infrared code.\n");
  fprintf(stderr, "  -t 'json'\tTransfer Infrared code.\n");
  fprintf(stderr, "  -t \"$(cat XXX.json)\"\n");
  fprintf(stderr, "  -t \"`cat XXX.json`\"\n");
  fprintf(stderr, "  -v       \tVerbose mode.\n");
}

void dump(const char* data)
{
	char * poi;
	char c;
	int i,j;
	int length = strlen(data);
	for(i = 0;i < length;i+=16)
	{
		printf("%04X ",i);
		for(j = 0;j < 16;j++)
		{
			if((i + j) >= length)
			{
				break;
			}
			c = data[i + j];
			printf("%02X ",(int)c & 0xFF);
			if(j == 7)
			{
			 printf("- ");
			}
		}
		for(/*j = 0*/;j < 16;j++)
		{
			printf("   ");
			if(j == 7)
			{
			 printf("- ");
			}
		}
		for(j = 0;j < 16;j++)
		{
			if((i + j) >= length)
			{
				break;
			}
			c = data[i + j];
			printf("%c",isprint(c) ? c : '?');
			if(j == 7)
			{
				printf(" ");
			}
		}
		for(/*j = 0*/;j < 16;j++)
		{
			printf(" ");
			if(j == 7)
			{
				printf(" ");
			}
		}
		printf("\n");
	}
}

