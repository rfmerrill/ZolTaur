/*
 * Button library by Aut
 * 
 */

typedef struct
{
   uint8_t pinNumber;
   uint8_t buttonDebounce;
   uint8_t debouncePerMs;
   //only gets changed by the external interrupt
   //thus the volatile
   volatile bool active;
   uint32_t lastRead;
}Button;


// Init with the addr to a button struct and a pin number to attach to
void buttonInit(Button *b, uint8_t pinNumber)
{
   b->pinNumber = pinNumber;
   pinMode(b->pinNumber, INPUT);
   b->debouncePerMs = 1;
   b->active = false;
   b->buttonDebounce = 0;
   b->lastRead = millis();
}

bool buttonRead(Button *b)
{
   return digitalRead(b->pinNumber);
}

void clearDebounce(Button *b)
{
   b-> buttonDebounce = 0;
}

bool debounce(Button *b)
{
   bool confirm = false;
   uint32_t timeRead = millis();
   if((timeRead - b->lastRead)>=10)
   {
      b->lastRead = timeRead;
      
      /* if(b->active)
      {
         if(buttonRead(b) == 1)
         {
            b->buttonDebounce++;
         }
         else
         {
            b->buttonDebounce--;
         }
      } */
      
      if(buttonRead(b) == 1)
      {
         b->buttonDebounce++;
      }
      else
      {
         b->buttonDebounce--;
      }
      
   }
   if(b->buttonDebounce >= 8)
   {
      confirm = true;
      b->active = false;
      clearDebounce(b);
   }
   
   if(b->buttonDebounce <= -4)
   {
      confirm = false;
      b->active = false;
      clearDebounce(b);
   }
   
   return confirm;
}

bool debounceLow(Button *b)
{
   //Looks for a string of 0's
   //To confirm a switch turning off
   bool confirm = false;
   uint32_t timeRead = millis();
   if((timeRead - b->lastRead)>=10)
   {
      b->lastRead = timeRead;
      
      if(buttonRead(b) == 1)
      {
         b->buttonDebounce++;
      }
      else
      {
         b->buttonDebounce--;
      }
   }
   
   if(b->buttonDebounce >= 4)
   {
      confirm = false;
      b->active = false;
      clearDebounce(b);
   }
   
   if(b->buttonDebounce <= -8)
   {
      confirm = true;
      b->active = false;
      clearDebounce(b);
   }
   
   return confirm;
}