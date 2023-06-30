#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <stdlib.h>

// struct to represent a node in the linked list
typedef struct Node {
  char* processName;
  struct Node* next;
} Node;

// head of the linked list that will hold the process names
Node* processList = NULL;

// add a new process name to the list
void addProcToList(char* processName) {
  Node* newNode = malloc(sizeof(Node));
  newNode->processName = malloc(strlen(processName) + 1);
  strcpy(newNode->processName, processName);
  printf("%s\n", processName);
  newNode->next = processList;
  processList = newNode;
}

// load process names from a file
void loadProcList() {
  HANDLE hFile;
  DWORD dwBytesRead = 0;
  char readBuffer[256] = {0};

  hFile = CreateFile(TEXT("processes.txt"),  // open processes.txt
                      GENERIC_READ,          // open for reading
                      0,                     // do not share
                      NULL,                  // default security
                      OPEN_EXISTING,         // existing file only
                      FILE_ATTRIBUTE_NORMAL, // normal file
                      NULL);                 // no attr. template

  if (hFile == INVALID_HANDLE_VALUE) {
    printf("could not open processes.txt.");
    return;
  }

  // read one line at a time, and add the process name to the list
  while (ReadFile(hFile, readBuffer, 255, &dwBytesRead, NULL)) {
    if (dwBytesRead == 0) {
      break;
    }
    readBuffer[strcspn(readBuffer, "\n")] = '\0';
    addProcToList(readBuffer);
    ZeroMemory(readBuffer, 256);
  }

  CloseHandle(hFile);
}

// check if a process name is in the list
int isProcInList(const char* process) {
  for (Node* node = processList; node != NULL; node = node->next) {
    if (strcmp(process, node->processName) == 0) {
      return 1;
    }
  }

  return 0;
}

// enumerate all running processes
void enumProcs() {
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
  if (hSnap == INVALID_HANDLE_VALUE) {
    printf("CreateToolhelp32Snapshot failed\n");
    return;
  }

  PROCESSENTRY32 pe32;
  pe32.dwSize = sizeof(PROCESSENTRY32);

  if (!Process32First(hSnap, &pe32)) {
    printf("Process32First failed\n");
    CloseHandle(hSnap);
    return;
  }

  do {
    if (isProcInList(pe32.szExeFile)) {
      printf("process found: %s\n", pe32.szExeFile);
    }
  } while (Process32Next(hSnap, &pe32));

  CloseHandle(hSnap);
}

// clean up the memory used by the list
void cleanup() {
  Node* current = processList;
  while (current != NULL) {
    Node* next = current->next;
    free(current->processName);
    free(current);
    current = next;
  }
}

int main() {
  loadProcList();
  enumProcs();
  cleanup();

  return 0;
}
