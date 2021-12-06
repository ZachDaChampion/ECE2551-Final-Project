#include "Memory.h"

#include <EEPROM.h>
#include <Arduino.h>
const int startofcontact = 21;
const int startofmessage = 174;
unsigned char arra [7]= {0xC0,0xFF,0xEE,0xFA,0xCE,0xCA,0x11};
unsigned int offcounter = 0;

    Memory::Memory()
    {
      
   
      if (this->hasSchema()==true)
      {
        
      }
      else
      {
      setSchema();
      this->clearContacts();
      this->clearMessages();
      }
      
      Contact node;
      saveNodeInformation(node);

      //nodes contact
     
    }
    Memory::Memory(Contact node)
    {
    
      if (this->hasSchema()==true)
      {
        
      }
      else
      {
      setSchema();
      this->clearContacts();
      this->clearMessages();
      
      saveNodeInformation(node);

      //nodes contact
      
      
      }
       
    }
    unsigned char* Memory::getNodeUUID()
    {
      
      unsigned char ptr[5];
      for(unsigned int i  = 0; i<5;i++)
      {
        
        ptr[i] = EEPROM.get((byte)(3+i),ptr[i]);
        //Serial.println(EEPROM.get((byte)(3+i),ptr[i]));
      }
      unsigned char* ptr1 = ptr;
      return ptr1;
    }
    char* Memory::getNodeName()
    {
      char ptr[10];
      for(unsigned int i  = 0; i<10;i++)
      {
        
          //Serial.println(x.read(8+i));
           ptr[i] = EEPROM.get((byte)(8+i),ptr[i]);
        
          
        
      }
      char* ptr1 = ptr;
      return ptr1;
    }
    unsigned short Memory::getNumberContacts()
    {
      unsigned short i;
      return EEPROM.get(20,i);
    }
    unsigned short Memory::getNumberMessages()
    {
      unsigned short i;
      return EEPROM.get(173,i);
    
    }
    Contact Memory::getContact(unsigned short index)
    {
      unsigned int count = 0;
      unsigned char ptrid [5];
       char ptrname[10];
       Contact x;
      if(index>EEPROM.read(20))
      {
        Contact y;
        return y;
      }
      else
      {
      for (unsigned int i = 21; i <= ((EEPROM.read(20)-1)*15)+21;i+=15)
      {
        if(i==(((index-1)*15)+21))
        {
          Contact yo = EEPROM.get(i,x);
          return yo;
          /*for(unsigned int j = 0; j < 15;j++)
          {
            if(j<5)
            {
              ptrid[j]= EEPROM.get(i+j,ptrid[j]);
              
            }
            else
            {
            ptrname[count]= EEPROM.get(i+j,ptrname[count]);
            
            count++;
            }
           
          }*/
        }
      }
      //char* ptrname1 = ptrname;
      //unsigned char* ptrid1 = ptrid;
      
      //Contact f(ptrid1,ptrname1);
      //Serial.println(f.getName());
      
      return;
      }
      
    }
    Message Memory::getMessage(unsigned short index)
    {
      unsigned int count=0;
    
      unsigned char ptrfrom[5];
      unsigned char ptrto[5];
      unsigned char leng;
      unsigned short payload;
      Message c;
      Serial.println(index);
      
      if(index>(EEPROM.read(173)))
      {
         
        Message y;
      
        return y;
      }
      else
      {
       
      for (unsigned int i = 174; i <= (((EEPROM.read(173)-1)*13)+174);i+=13)
      {
       
        if(i==((index-1)*13)+174)
        {
          EEPROM.get(i,c);
          
          return c;
         /*for(unsigned int j = 0; j < 13;j++)
           {
            if(j<5)//from
            {
            ptrfrom[j]= EEPROM.get((byte)i+j,ptrfrom[j]);
            
            }
            else if(j<10)//to
            {
             ptrto[count]= EEPROM.get((byte)i+j,ptrto[count]);
            //*(ptrto+count)= x.read(index+i);
            count++;
            }
            else if(j==10)//pay
            {
             payload |= EEPROM.read(i+j)<<8;
            }
            else if(j==11)
            {
             payload |= (byte)EEPROM.read(i+j);
            }
            else//length
            {
            leng = EEPROM.read(j+i);
            }
           
          }*/
        }
      }
      //unsigned char* to = ptrto;
      //unsigned char* from = ptrfrom;
      //Message y(from,to,payload,leng);
      
      }
      return;
    }
    bool Memory::saveContact(Contact contact)
    {
      
       int count = 0;
      unsigned char* ptrid = contact.getUUID();
      
      char* ptrname = contact.getName();
      
      
      unsigned int newspace = startofcontact+(EEPROM.read(20)*15);
      if(10==EEPROM.read(20))
      {
        
        return false;
      }
      else
      {
        EEPROM.put(newspace,contact);
      /*for (unsigned int i = 0; i < 15;i++)
      {
        if(i<5)
        {
          EEPROM.put(i+newspace,*(ptrid+i));
          
          count = 0;
        }
        else
        {
          EEPROM.put(i+newspace,*(ptrname+count));
          //
          count++;
        }
      }*/
         count = 1;
         count = (EEPROM.get(20,count))+1;
         EEPROM.put(20,count);
         
         
        return true;
      }
     
    }
    void Memory::saveMessage(Message message)
    {
      
      int count = 0;
      unsigned char* ptrfrom = message.getFrom();
      unsigned char* ptrto = message.getTo();
      unsigned char leng = message.getLength();
      unsigned short payload = message.getPayload();
      unsigned int newspace = getMessagePointerOffset();
      if(20==offcounter)
      {
        offcounter = 0;
        newspace = getMessagePointerOffset();
        offcounter = 1;
      }
      else
      {
        offcounter++;
      }
      EEPROM.put(newspace,message);
      
      /*for (unsigned int i = 0; i < 13;i++)
      {
        if(i<5)
        {
          EEPROM.put(i+newspace,*(ptrfrom+i));
          
          count = 0;
        }
        else if(i<10)
        {
          EEPROM.put(i+newspace,*(ptrto+count));
          
          count++;
        }
        else if(i==10)
        {
          EEPROM.put(i+newspace,(unsigned char)payload>>8);
        }
        else if(i==10)
        {
          EEPROM.put(i+newspace,(unsigned char)payload);
        }
        else
        {
          EEPROM.put(i+newspace,leng);
        }
      
        
        
      }*/
      if(20<=EEPROM.read(173))
      {
        
      }
      else
      {
        count = EEPROM.get(173,count)+1;
        EEPROM.put(173,count);
      }
      
    }
    void Memory::saveNodeInformation(Contact contact)
    {
      unsigned char* ptr = contact.getUUID();
      
      for(unsigned int j  = 0; j<5;j++)
      {
        
        EEPROM.put(3+j,*(ptr+j));
        //Serial.println(EEPROM.read(3+j));
      }
      char* ptr1 = contact.getName();
      for(int j  = 0; j<10;j++)
      {
        
        
        EEPROM.put(8+j,(byte)(*(ptr1+j)));
        //Serial.println(EEPROM.read(8+j));
        
        
        
      }
    }
    void Memory::reset()
    {
      clearMessages();
      clearContacts();
    }
       

  //protected:
    bool Memory::hasSchema()
    {
      bool corupt = false;
        int count = 0;
      for (unsigned int i = 0;i<173;i++)
      {
        if(i<3)
        {
          if(EEPROM.read(i)==arra[count])
          {
            
          }
          else
          {
            corupt = true;
          }
          count++;
        }
        else if(i<20&&i>17)
        {
          if(EEPROM.read(i)==arra[count])
          {
            
          }
          else
          {
            corupt = true;
          }
          count++;
        }
        else if(i>170)
        {
          if(EEPROM.read(i)==arra[count])
          {
            
          }
          else
          {
            corupt = true;
          }
          count++;
        }
      }

      return !(corupt);
    }
    void Memory::setSchema()
    {
        int count = 0;
      for (unsigned int i = 0;i<173;i++)
      {
        if(i<3)
        {
          EEPROM.put((byte)i,arra[count]);
          count++;
        }
        else if(i<20&&i>17)
        {
          EEPROM.put((byte)i,arra[count]);
          count++;
        }
        else if(i>170)
        {
          EEPROM.put((byte)i,arra[count]);
          count++;
        }
      }
    }
    void Memory::clearMessages()
    {
         for(unsigned int i = 173; i<434; i++)
      {
        EEPROM.put((byte)i,(byte)0);
      }
    }
    void Memory::clearContacts()
    {
      for(unsigned int i = 20; i<171; i++)
      {
        
        
          EEPROM.put((byte)i,(byte)0);
        
      }
    }
    unsigned short Memory::getMessagePointerOffset()
    {
      
        
        return startofmessage + (offcounter*13);
      
    
    }
    // Add as you see fit
