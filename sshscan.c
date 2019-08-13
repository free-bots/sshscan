/* this program listens on the default ssh port
 * if devices are found in the arp cache 
 * then they are displayed in the console
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
// 1 for debugging messages
#define DEBUG 0
// colors in console print
#define GREEN "\x1b[32m"
#define MAGENTA "\x1b[35m"
#define RED "\x1B[31m"
// length of one line
#define LINE_SIZE 512
// length for open item in a column
#define LINE_CHARACTER_SIZE 256
#define LINEBREAK '\n'
// path of the arp cache
#define arp "/proc/net/arp"
// the default ssh port
#define PORT 22

int getLineSize();
int isPortOpen(char *address[], int port);

int main() {
    // get the length of the cache
    int lines = getLineSize(arp);
    if (lines == -1) {
        printf(RED "no devices found\n");
        return 0;
    }
    // save the found devices here
    char *ips[lines];
    char *macs[lines];
    char *interfaces[lines];
    // clean the memory
    memset(ips, 0, sizeof(ips));
    memset(macs, 0, sizeof(macs));
    memset(interfaces, 0, sizeof(interfaces));

    printf("\n"
           MAGENTA "Searching for addresses...\n"
           "\n");
    // open the arp cache
    FILE* file = fopen(arp, "r");
    if (file == NULL) {
        printf(RED "no devices found\n");
        return 1;
    }

    int size = LINE_CHARACTER_SIZE;

    int position = -1;
    // read each line
    char line[LINE_SIZE];
    while (fgets(line, LINE_SIZE, file)) {
        char ip_address[size];
        int hw_type;
        int flags;
        char mac_address[size];
        char mask[size];
        char device[size];

        position++;
        if (position == 0) {
            // ignore the first line
            continue;
        }
        position--;
        // read the arp cache
        sscanf(line, "%s 0x%x 0x%x %s %s %s\n",
               ip_address,
               &hw_type,
               &flags,
               mac_address,
               mask,
               device);
        // copy to fix references
        ips[position] = strdup(ip_address);
        macs[position] = strdup(mac_address);
        interfaces[position] = strdup(device);
        position++;   
         
    }
    fclose(file);
    if (position == -1) {
         printf(RED "no devices found\n");
    }
    for (int i = 0; i < lines; i++) {
        printf(GREEN "Found device with IP: %s %15s %s MAC: %s %15s %s on interface: %s %15s %s\n",
               MAGENTA,
               ips[i],
               GREEN,
               MAGENTA,
               macs[i],
               GREEN,
               MAGENTA,
               interfaces[i],
               GREEN);
    }
    printf("\n"
           "Connecting with devices...\n" 
           "\n");
    int found = -1;
    for (int i = 0; i < lines; i++) {
        if (isPortOpen(ips[i], PORT) > 0){
             printf(GREEN "Device with IP: %s %15s %s listens on port: %s %5d %s\n",
               MAGENTA,
               ips[i],
               GREEN,
               MAGENTA,
               PORT,
               GREEN);
               found++;
        }
    }
    if (found < 0) {
        printf(RED "no device listen on port %d\n", PORT);
    }
    return 0;
}

/* get the amount of lines in a file
 * return -1 if error
 */ 
int getLineSize(char* path) {
    int lineSize = -1;
    FILE* f = fopen(path, "r");
    if (f == NULL) {
        return -1;
    }
    while(!feof(f)) {
        char c = fgetc(f);
        if (c == LINEBREAK) {
            lineSize++;
        }
    }
    fclose(f);
    return lineSize;
}

/* open a socket to check if port an is open
 * 1 = open -1 = error or closed
 */
int isPortOpen(char *address[], int port) {
    int sock_fd;
    struct sockaddr_in server_addr;
    // create ipv4 tcp socket
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(sock_fd < 0) {
        if (DEBUG) {
            perror("socket");
        }
      // not found
      return -1;
    }
    memset((void *) &server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(address);
    server_addr.sin_port = htons(port);
    int status;
    status = connect(sock_fd, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if(status < 0) {
        if (DEBUG) {
            perror("connect");    
        }
      // refused
      return -1;
    }
    close(sock_fd);
    // connected
    return 1;
}