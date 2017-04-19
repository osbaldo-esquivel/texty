#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<termios.h>
#include<ctype.h>
#include<errno.h>
#include<sys/ioctl.h>
#define CTRL_KEY(k) ((k) & 0x1f)

void enterRaw();
void noRaw();
void killExit(const char *);
char readKeypress();
void processKeypress();
void clearScreen();
void initRows();
void textyInit();
int sizeWin(int *,int *);
int cursorPos(int *, int *);

struct config {
  struct termios termInit;
  int winRow;
  int winCol;
};

struct config C;

int main() {
  // go into raw mode
  enterRaw();
  // initialize editor
  textyInit();
  // read characters typed in
  while(1) {
    clearScreen();
    processKeypress();
  }

  return 0;
}

void killExit(const char *kill) {
  perror(kill);
  exit(1);
}

void noRaw() {
  // set the original terminal attributes
  if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&C.termInit) == -1) {
    killExit("tcsetattr");
  }
}

void enterRaw() {
  // get terminal attributes
  if(tcgetattr(STDIN_FILENO, &C.termInit) == -1) {
    killExit("tcgetattr");
  }
  // set original attributes at exit
  atexit(noRaw);
  struct termios goRaw = C.termInit;
  // bitwise-AND the bitwise-NOT of
  // EXON,CRNL flags
  goRaw.c_iflag &= ~(IXON|ICRNL|BRKINT|INPCK|ISTRIP);
  // bitwise-AND the bitwise-NOT of
  // ECHO,ICANON,ISIG flags
  goRaw.c_lflag &= ~(ECHO|ICANON|ISIG|IEXTEN);
  // turn off OPOST flag
  goRaw.c_oflag &= ~(OPOST);
  goRaw.c_cflag |= (CS8);
  goRaw.c_cc[VMIN] = 0;
  goRaw.c_cc[VTIME] = 1;
  // set terminal attributes
  if(tcsetattr(STDIN_FILENO,TCSAFLUSH,&goRaw) == -1) {
    killExit("tcsetattr");
  }
}

char readKeypress() {
  char aCh;
  int temp;

  while((temp = read(STDIN_FILENO,&aCh,1)) != 1) {
    if(temp == -1 && errno != EAGAIN) {
      killExit("read");
    }
  }
  return aCh;
}

void processKeypress() {
  char tempCh = readKeypress();

  switch(tempCh) {
    case CTRL_KEY('q'):
      write(STDOUT_FILENO,"\x1b[2J",4);
      write(STDOUT_FILENO,"\x1b[H",3);
      exit(0);
      break;
  }
}

void clearScreen() {
  write(STDOUT_FILENO,"\x1b[2J",4);
  write(STDOUT_FILENO,"\x1b[H",3);
  initRows();
  write(STDOUT_FILENO,"\x1b[H",3);
}

void initRows() {
  int i;
  for(i=0;i<C.winRow;i++) {
    write(STDOUT_FILENO,"~\r\n",3);
  }
}

int sizeWin(int *col, int *row) {
  struct winsize size;

  if(ioctl(STDOUT_FILENO,TIOCGWINSZ,&size) == -1 || size.ws_col == 0) {
    return cursorPos(row,col);
  }
  else {
    *row = size.ws_col;
    *col = size.ws_row;
    return 0;
  }
}

void textyInit() {
  if(sizeWin(&C.winRow,&C.winCol) == -1) {
    killExit("sizeWin");
  }
}

int cursorPos(int *r, int *c) {
  char buffer[32];
  unsigned int i = 0;

  if(write(STDOUT_FILENO,"\x1b[6n",4) != 4) {
    return -1;
  }

  while(i < sizeof(buffer) - 1) {
    if(read(STDIN_FILENO,&buffer[i],1) != 1) {
      break;
    }
    if(buffer[i] == 'R') {
      break;
    }
    i++;
  }

  buffer[i] = '\0';

  printf("\r\n&buffer[1]: '%s'\r\n",&buffer[1]);
  char ch;

  readKeypress();
  return -1;
}
