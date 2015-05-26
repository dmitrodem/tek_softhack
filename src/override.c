#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <linux/i2c.h>
#include <stdlib.h>

#include <stdio.h>

#define _GNU_SOURCE
#define __USE_GNU
#include <dlfcn.h>

ssize_t (*orig_read)(int fd, void *buf, size_t count);
int (*orig_ioctl)(int fd, unsigned long request, char *argp);

int is_i2c(int fd){
    char path[1024];
    char lbuf[4096];
    char i2c_dev[] = "/dev/i2c-0";

    sprintf(path, "/proc/self/fd/%i", fd);

    ssize_t filepath_len = readlink(path, lbuf, sizeof(lbuf));
    lbuf[filepath_len] = '\0';
    return strcmp(lbuf, i2c_dev) == 0 ? 1: 0;
}


struct appfunction_t {
    pid_t pid;
    int selected;
    char app_name[16];
    unsigned int i2c_addr;
    char app_envkey[1024];
};

static struct appfunction_t app[] = {
    {
        .pid = 0,
        .selected = 0,
        .app_name = "DPO2COMP",
        .i2c_addr = 0x50,
        .app_envkey = "DPO0"
    },
    {
        .pid = 0,
        .selected = 0,
        .app_name = "DPO2EMBD",
        .i2c_addr = 0x51,
        .app_envkey = "DPO1"
    },
    {
        .pid = 0,
        .selected = 0,
        .app_name = "DPO2AUTO",
        .i2c_addr = 0x52,
        .app_envkey = "DPO02"
    },
    {
        .pid = 0,
        .selected = 0,
        .app_name = "DPO2VID",
        .i2c_addr = 0x53,
        .app_envkey = "DPO03"
    }
};

const static size_t app_len = sizeof(app)/sizeof(app[0]);

ssize_t read(int fd, void *buf, size_t count){
    ssize_t result = orig_read(fd, buf, count);

    if (is_i2c(fd)){
        size_t i;
        size_t index = -1;
        for (i = 0; i < app_len; i++){
            if ((app[i].pid == getpid()) && app[i].selected == 1){
                index = i;
                break;
            }
        }
        if (index != -1){
            printf("App selected: %s\n", app[index].app_name);
            if (count == 1){
                /* ping app -- just return success */
                result = 1;
            } else if (count == 15){
                /* return string descriptor */
                result = 15;
                int j;
                for (j = 0; j < 15; j++){
                    ((char *)buf)[j] = app[index].app_name[j];
                }
            } else {
                printf("Unhandled read request of %i bytes!\n", count);
            }
        }
    }
    
    return result;
}

static void select_app(size_t index, pid_t pid){
    size_t i;
    for (i = 0; i < app_len; i++){
        /* only change settings for specified process */
        if (app[i].pid == pid){
            if (index == i){
                app[i].selected = 1;
            } else {
                app[i].selected = 0;
            }
        }
    }
}

int ioctl(int fd, unsigned long request, char *argp){
    unsigned int addr;
    int result = orig_ioctl(fd, request, argp);
    if (is_i2c(fd)){
        switch(request){
            case I2C_SLAVE:
                addr = (unsigned int)argp;
                size_t i;
                for (i = 0; i < app_len; i++){
                    if (app[i].i2c_addr == addr){
                        /* address matched -- marking it in the table */
                        app[i].pid = getpid();
                        select_app(i, getpid());
                        break;
                    }
                }
                if (i == app_len){
                    /* no one selected -- disabling all indicies */
                    int j;
                    for (j = 0; j < app_len; j++){
                        app[j].selected = 0;
                    }
                }
                break;
            default:
                break;
        }
    }
    return result;
}

void _init(void) {
    /* initialize app descriptors if any */
    char *envkey;
    int i;
    for (i = 0; i < app_len; i++){
        envkey = getenv(app[i].app_envkey);
        if (envkey){
            strcpy(app[i].app_name, envkey);
            printf("Setting 0x%02x to \"%s\"\n", app[i].i2c_addr, app[i].app_name);
        }
    }
    orig_read = dlsym(RTLD_NEXT, "read");
    orig_ioctl = dlsym(RTLD_NEXT, "ioctl");
}










