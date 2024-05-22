#ifndef BUTTON_H
#define BUTTON_H

void enableAllButtons();
void enableButton(int button);

void enableButtonInterrupt(int button);
void enableAllButtonInterrupts();

int buttonPushed(int button);
int buttonReleased(int button);

#endif