/*
 * 8 Channel Simutaneous ADC Sampling
 *
 * ECE4012: Senior Design, Group: WS2
 * Members: Yao Lu
 * Feb 13, 2014
 *
 * Author: Yao Lu
 *
 * buffer.h
 *
 * Templated buffer class for PRIMATIVE data types ONLY.
 * Used for managing raw data buffer and channel buffers
 * within the FT232H class.
 *
 */


template <class T>
class circularBuffer {
  private:
    T* buffer;
    int size;          // max size of buffer
    int head;          // location of first element
    int entries;       // number of entries
    int current;
  public:
    circularBuffer(){
        size=head=entries=current=0;
    }
    circularBuffer(int size){
        this->size = size;
        head=entries=current=0;
        buffer = new T[size];
    }
    void setSize(int size){
        this->size = size;
        buffer = new T[size];
    }
    void reset(){ current = 0; }
    int getEntries(){ return entries; }
    /* Gets the next item until the end is reached */
    bool getNext(T& item){
        if(current == -1 || entries == 0) return false;
        item = buffer[current];
        current = (current+1)%size;
        if(current == head) current = -1;
        return true;
    }
    /* Retrieve first element and remove from buffer */
    bool pop(T* item){
        if(entries < 1) return false;
        *item = buffer[head];
        head = (head+1)%size;
        entries--;
        return true;
    }
    /* Store the first N elements but don't remove from buffer */
    bool getN(T* buf, int n){
        if(n > entries) return false;
        for(int i = 0; i < n; i++){
            buf[i] = buffer[(head+i)%size];
        }
        return true;
    }
    /* Remove the first N elements from buffer */
    bool clearN(int n){
        if(n > entries) return false;
        head = (head+n)%size;
        entries -= n;
        return true;
    }
    /* Add an item to the end of the buffer */
    int add(T item){
        if(entries == size) head++;
        int index = (head+entries)%size;
        buffer[index] = item;
        if(entries != size) entries++;
        return index;
    }
    /* Add N items to the end of the buffer */
    void addN(T* buf, int n){
        for(int i = 0; i < n; i++){
            if(entries == size) head++;
            buffer[(head+entries)%size] = buf[i];
            if(entries != size) entries++;
        }
    }
    /* Overloaded indexing operator for easy access, no bound checking */
    T& operator[](int i){ return(buffer[(head+i)%size]); }
};

