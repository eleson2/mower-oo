#ifndef QUEUE_H
#define QUEUE_H

template < class Data, const char nofItems, unsigned char _Properties> 
class  Queue {
   static_assert (nofItems > 1 && nofItems<40,"Nof elements is unsupported"); 
public:
      char head = 0;  // nofItems Head of Queue, where data was last written
      char count = 0;  // Nmbr of items in Queue
protected:
    // bit 0  overwrite when new data comes in?
    // bit 1  etc 
    const char  properties = _Properties;
    Data data[nofItems];      // Simple Array that holds the queue.

// Methods

protected:
   inline char advancePtr(char p) {
      if (p >= nofItems-1) return 0;
      return p+1;
   };

public:
   inline bool dataAvailable() { return count; }; 

   inline bool spaceAvailable() { return (nofItems - count); };  

   constexpr bool canOverwrite() { return ((properties & 0x01) == 1); };

   constexpr int depth() { return nofItems; };

   bool  pull(Data &_data) {
      if (!dataAvailable()) return false;
      count-=1;
      char tail = head-count;
      if(tail < 0) tail +=nofItems;
      _data =  data[tail];
      return true;
    };

   bool push(Data _Data) { 
      auto Avail = spaceAvailable();
      if (Avail || canOverwrite()) { 
         head = advancePtr(head);
         data[head] = _Data;
      }
      else { 
         return false;
      };
      if(Avail) count+=1; 
      return true;
   };
};
#endif