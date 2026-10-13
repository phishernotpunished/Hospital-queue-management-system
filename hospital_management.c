#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#define NAME_SIZE 100
#define INPUT_SIZE 256

typedef struct Patient {
    int token;
    char name[NAME_SIZE];
    int age;
    int emergency;
    struct Patient *next;
} Patient;

Patient *front = NULL;
Patient *rear = NULL;
Patient *history = NULL;
Patient *historyRear = NULL;

int nextToken = 1;
int totalServed = 0;

void readLine(const char *prompt, char *buffer, size_t size) {
    printf("%s", prompt);

    if (fgets(buffer, (int)size, stdin) == NULL) {
        printf("\nInput ended. Exiting.\n");
        exit(0);
    }

    size_t len = strlen(buffer);

    if (len > 0 && buffer[len - 1] == '\n') {
        buffer[len - 1] = '\0';
    } else {
        int ch;
        while ((ch = getchar()) != '\n' && ch != EOF) {
        }
    }
}

int readInt(const char *prompt, int min, int max) {
    char buffer[INPUT_SIZE];

    while (1) {
        char *end;
        long value;

        readLine(prompt, buffer, sizeof(buffer));

        errno = 0;
        value = strtol(buffer, &end, 10);

        while (*end == ' ' || *end == '\t' || *end == '\r') {
            end++;
        }

        if (buffer[0] != '\0' &&
            end != buffer &&
            *end == '\0' &&
            errno != ERANGE &&
            value >= min &&
            value <= max) {
            return (int)value;
        }

        printf("Invalid input. Enter a number from %d to %d.\n",
               min, max);
    }
}

Patient *createPatient(int token, const char *name,
                       int age, int emergency) {
    Patient *p = malloc(sizeof(Patient));

    if (p == NULL) {
        printf("Memory allocation failed.\n");
        return NULL;
    }

    p->token = token;
    snprintf(p->name, sizeof(p->name), "%s", name);
    p->age = age;
    p->emergency = emergency;
    p->next = NULL;

    return p;
}

void printPatient(const Patient *p) {
    printf("Token: %-5d | Name: %-22s | Age: %-3d | Type: %s\n",
           p->token,
           p->name,
           p->age,
           p->emergency ? "EMERGENCY" : "Normal");
}

void enqueuePatient(void) {
    char name[NAME_SIZE];
    int age, emergency;

    readLine("Enter patient name: ", name, sizeof(name));

    if (name[0] == '\0') {
        printf("Patient name cannot be empty.\n");
        return;
    }

    age = readInt("Enter patient age (0-120): ", 0, 120);

    printf("1. Normal\n");
    printf("2. Emergency\n");

    emergency = readInt("Select patient type: ", 1, 2);

    Patient *newPatient = createPatient(
        nextToken, name, age, emergency == 2
    );

    if (newPatient == NULL) {
        return;
    }

    if (front == NULL) {
        front = rear = newPatient;
    } else if (newPatient->emergency && !front->emergency) {
        newPatient->next = front;
        front = newPatient;
    } else {
        Patient *temp = front;

        while (temp->next != NULL &&
               (temp->next->emergency || !newPatient->emergency)) {
            temp = temp->next;
        }

        newPatient->next = temp->next;
        temp->next = newPatient;

        if (newPatient->next == NULL) {
            rear = newPatient;
        }
    }

    printf("Patient registered successfully!\n");
    printf("Assigned token number: %d\n", nextToken);

    if (emergency == 2) {
        printf("Priority: EMERGENCY\n");
    } else {
        printf("Priority: Normal\n");
    }

    nextToken++;
}

void displayQueue(void) {
    Patient *temp = front;
    int position = 1;

    if (front == NULL) {
        printf("No patients are waiting.\n");
        return;
    }

    printf("Patient Waiting Queue\n");

    while (temp != NULL) {
        printf("%d. ", position++);
        printPatient(temp);
        temp = temp->next;
    }
}

void searchPatient(void) {
    int token = readInt("Enter patient token number: ", 1, INT_MAX);
    Patient *temp = front;

    while (temp != NULL) {
        if (temp->token == token) {
            printf("Patient found in waiting queue:\n");
            printPatient(temp);
            return;
        }

        temp = temp->next;
    }

    printf("Patient with token %d is not in the waiting queue.\n",
           token);
}

void cancelPatient(void) {
    int token = readInt("Enter token number to cancel: ", 1, INT_MAX);

    Patient *temp = front;
    Patient *previous = NULL;

    while (temp != NULL && temp->token != token) {
        previous = temp;
        temp = temp->next;
    }

    if (temp == NULL) {
        printf("Patient not found in the waiting queue.\n");
        return;
    }

    if (previous == NULL) {
        front = temp->next;
    } else {
        previous->next = temp->next;
    }

    if (temp == rear) {
        rear = previous;
    }

    printf("Registration cancelled for patient: %s (Token %d)\n",
           temp->name, temp->token);

    free(temp);

    if (front == NULL) {
        rear = NULL;
    }
}

void addToHistory(const Patient *p) {
    Patient *copy = createPatient(
        p->token, p->name, p->age, p->emergency
    );

    if (copy == NULL) {
        printf("Warning: Could not save this patient in history.\n");
        return;
    }

    if (history == NULL) {
        history = historyRear = copy;
    } else {
        historyRear->next = copy;
        historyRear = copy;
    }
}

void callNextPatient(void) {
    if (front == NULL) {
        printf("No patients are waiting for consultation.\n");
        return;
    }

    Patient *served = front;
    front = front->next;

    if (front == NULL) {
        rear = NULL;
    }

    printf("Now in consultation:\n");
    printPatient(served);
    printf("Patient has been called for consultation.\n");

    addToHistory(served);
    totalServed++;

    free(served);
}

void displayHistory(void) {
    Patient *temp = history;

    if (history == NULL) {
        printf("No patients have been served yet.\n");
        return;
    }

    printf("Consultation History\n");

    while (temp != NULL) {
        printPatient(temp);
        temp = temp->next;
    }
}

int countWaitingPatients(void) {
    int count = 0;
    Patient *temp = front;

    while (temp != NULL) {
        count++;
        temp = temp->next;
    }

    return count;
}

void displayStatistics(void) {
    int waiting = countWaitingPatients();
    int emergencyCount = 0;
    int normalCount = 0;

    Patient *temp = front;

    while (temp != NULL) {
        if (temp->emergency) {
            emergencyCount++;
        } else {
            normalCount++;
        }

        temp = temp->next;
    }

    printf("Patients currently waiting: %d\n", waiting);
    printf("Emergency patients waiting: %d\n", emergencyCount);
    printf("Normal patients waiting: %d\n", normalCount);
    printf("Patients served: %d\n", totalServed);
}

void freeList(Patient **list) {
    Patient *temp = *list;

    while (temp != NULL) {
        Patient *next = temp->next;
        free(temp);
        temp = next;
    }

    *list = NULL;
}

void cleanup(void) {
    freeList(&front);
    freeList(&history);

    rear = NULL;
    historyRear = NULL;
}

int main(void) {
    int choice;

    printf("Hospital Queue Management System\n");

    while (1) {
        printf("\n1. Register a new patient\n");
        printf("2. Display waiting queue\n");
        printf("3. Call next patient\n");
        printf("4. Search patient by token\n");
        printf("5. Cancel patient registration\n");
        printf("6. View consultation history\n");
        printf("7. View queue statistics\n");
        printf("0. Exit\n");

        choice = readInt("Enter your choice: ", 0, 7);

        switch (choice) {
            case 1:
                enqueuePatient();
                break;

            case 2:
                displayQueue();
                break;

            case 3:
                callNextPatient();
                break;

            case 4:
                searchPatient();
                break;

            case 5:
                cancelPatient();
                break;

            case 6:
                displayHistory();
                break;

            case 7:
                displayStatistics();
                break;

            case 0:
                cleanup();
                printf("Exiting Hospital Queue Management System.\n");
                return 0;
        }
    }
}
