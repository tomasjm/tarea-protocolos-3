#include <stdio.h>
#include <stdlib.h>
#include "menu.h"

void clearScreen() {
  system("clear");
}

void printMenu() {
  clearScreen();
  printf("Main menu\n");
  printf("Please select an option.\n");
  printf("1. Send telemetry to a node \n");
  printf("2. Close program\n");
}

void getOptionAndValidate(int *optPtr) {
  scanf("%d", optPtr);
  while(*optPtr < 1 || *optPtr > 2) {
    printMenu();
    printf("Please select a valid option...\n");
    getOptionAndValidate(optPtr);
  }
}
