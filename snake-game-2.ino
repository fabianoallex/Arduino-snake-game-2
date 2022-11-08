/*************************************************************************************************************
*******************************LEDCONTROL ALTERADA************************************************************
**************************************************************************************************************
mais informações aqui: http://fabianoallex.blogspot.com.br/2015/09/arduino-alteracoes-na-biblioteca.html
**************************************************************************************************************/

//the opcodes for the MAX7221 and MAX7219
#define OP_DECODEMODE  9
#define OP_INTENSITY   10
#define OP_SCANLIMIT   11
#define OP_SHUTDOWN    12
#define OP_DISPLAYTEST 15

class LedControl {
  private :
    byte spidata[16];
    byte * status;
    int SPI_MOSI;
    int SPI_CLK;
    int SPI_CS;
    int maxDevices;
    int _auto_send;
    
    void spiTransfer(int addr, volatile byte opcode, volatile byte data) {
      int offset   = addr*2;
      int maxbytes = maxDevices*2;
      for(int i=0;i<maxbytes;i++)  { spidata[i]=(byte)0; }
      spidata[offset+1] = opcode;
      spidata[offset]   = data;
      
      digitalWrite(SPI_CS,LOW);
      for(int i=maxbytes;i>0;i--) { shiftOut(SPI_MOSI,SPI_CLK,MSBFIRST,spidata[i-1]); }
      digitalWrite(SPI_CS,HIGH);
    }
      
  public:
    LedControl(int dataPin, int clkPin, int csPin, int numDevices) {
      _auto_send  = true;
      SPI_MOSI    = dataPin;
      SPI_CLK     = clkPin;
      SPI_CS      = csPin;
      maxDevices  = numDevices;
      
      pinMode(SPI_MOSI, OUTPUT);
      pinMode(SPI_CLK,  OUTPUT);
      pinMode(SPI_CS,   OUTPUT);
      digitalWrite(SPI_CS, HIGH);
      
      status      = new byte[maxDevices * 8]; //instancia o array de acordo com a quantia de displays usados
      for(int i=0;i<maxDevices * 8 ;i++) { status[i]=0x00; }
      
      for(int i=0;i<maxDevices;i++) {
        spiTransfer(i, OP_DISPLAYTEST,0);
        setScanLimit(i, 7);               //scanlimit is set to max on startup
        spiTransfer(i, OP_DECODEMODE,0);  //decode is done in source
        clearDisplay(i);
        shutdown(i,true);                 //we go into shutdown-mode on startup
      }
    }
    
    void startWrite() {  _auto_send = false;  };
    
    void send() {
      for (int j=0; j<maxDevices; j++) {
        int offset = j*8;
        for(int i=0;i<8;i++) { spiTransfer(j, i+1, status[offset+i]); }
      }
      _auto_send = true;
    }
    
    int getDeviceCount(){ return maxDevices; }
    
    void shutdown(int addr, bool b){
      if(addr<0 || addr>=maxDevices) return;
      spiTransfer(addr, OP_SHUTDOWN, b ? 0 : 1);
    }
    
    void setScanLimit(int addr, int limit){
      if(addr<0 || addr>=maxDevices) return;
      if(limit>=0 && limit<8) spiTransfer(addr, OP_SCANLIMIT,limit);
    }
    
    void setIntensity(int addr, int intensity) {
      if(addr<0 || addr>=maxDevices)   {  return;                                    }
      if(intensity>=0 && intensity<16) {  spiTransfer(addr, OP_INTENSITY, intensity); }
    }
    
    void clearDisplay(int addr){
      if(addr<0 || addr>=maxDevices) return;
      
      int offset = addr*8;
      for(int i=0;i<8;i++) {
        status[offset+i] = 0;
        if (_auto_send) { spiTransfer(addr, i+1, status[offset+i]); }
      }
    }
    
    void setLed(int addr, int row, int column, boolean state) {
      if(addr<0 || addr>=maxDevices)             { return; }
      if(row<0 || row>7 || column<0 || column>7) { return; }
      
      int offset = addr*8;
      byte val = B10000000 >> column;
      
      if(state) { status[offset+row] = status[offset+row] | val; }
      else {
        val=~val;
        status[offset+row] = status[offset+row]&val;
      }
      
      if (_auto_send) { spiTransfer(addr, row+1, status[offset+row]); }
    }
    
    void setRow(int addr, int row, byte value) {
      if(addr<0 || addr>=maxDevices) return;
      if(row<0 || row>7) return;
      int offset = addr*8;
      status[offset+row] = value;
      if (_auto_send) {
        spiTransfer(addr, row+1, status[offset+row]);
      }
    }
    
    void setColumn(int addr, int col, byte value) {
      if(addr<0 || addr>=maxDevices) return;
      if(col<0 || col>7)             return;
      
      byte val;
      for(int row=0; row<8; row++) {
        val=value >> (7-row);
        val=val & 0x01;
        setLed(addr,row,col,val);
      }
    }
};
/*************************************************************************************************************
*******************************FIM LEDCONTROL ALTERADA********************************************************
**************************************************************************************************************/

/*************************************************************************************************************
*******************************BIT ARRAY *********************************************************************
**************************************************************************************************************
mais informações aqui: http://fabianoallex.blogspot.com.br/2015/09/arduino-array-de-bits.html
**************************************************************************************************************/

class BitArray{
  private:
    int _num_bits;   //quantidade de bits a serem gerenciados
    int _num_bytes;  //quantidade de bytes utilizados para armazenar os bits a serem gerenciados
    byte * _bytes;   //array de bytes onde estaram armazenados os bits
  public:
    BitArray(int num_bits){
      _num_bits  = num_bits;
      _num_bytes = _num_bits/8 + (_num_bits%8 ? 1 : 0) + 1;
      _bytes = (byte *)(malloc( _num_bytes * sizeof(byte) ) );
    }
    
    void write(int index, byte value) {
      byte b = _bytes[ index/8 + (index%8 ? 1 : 0) ];
      unsigned int bit = index%8;
      if (value) { b |= (1 << bit); } else { b &= ~(1 << bit);  }
      _bytes[ index/8 + (index%8 ? 1 : 0) ] = b;
    }
    
    void write(byte value) {
      for(int j=0; j<_num_bytes;j++) { _bytes[j] = value ? B11111111 : B00000000;  } 
    }
    
    int read(int index) {
      byte b = _bytes[ index/8 + (index%8 ? 1 : 0) ];
      unsigned int bit = index%8;
      return (b & (1 << bit)) != 0;
    }
    
    ~BitArray(){ free ( _bytes ); }
};


class BitArray2D {
  private:
    unsigned int _rows;
    unsigned int _columns;
    unsigned int _cols_array; //pra cada 8 colunas, 1 byte é usado 
    byte**       _bits;
  public:
    BitArray2D(unsigned int rows, unsigned int columns){
      _rows       = rows;
      _columns    = columns;
      _cols_array = columns/8 + (_columns%8 ? 1 : 0) + 1; //divide por 8 o número de colunas
      _bits = (byte **)malloc(_rows * sizeof(byte *));
      for(int i=0;i<_rows;i++){ _bits[i] = (byte *)malloc(  _cols_array  *  sizeof(byte)); } //cria varios arrays
      clear();
    }
    
    unsigned int rows(){ return _rows; }
    unsigned int columns(){ return _columns; }
    
    void clear() { 
      for(int i=0;i<_rows;i++){      
        for(int j=0; j<_cols_array;j++) { _bits[i][j] = B00000000; }       
      }   
    }
  
    void write(unsigned int row, unsigned int column, int value){
      byte b = _bits[row][ column/8 + (column%8 ? 1 : 0) ];
      unsigned int bit = column%8;
      
      if (value) { b |= (1 << bit); } else { b &= ~(1 << bit);  }
      
      _bits[row][ column/8 + (column%8 ? 1 : 0) ] = b;
    }
    
    void write(byte value){
      for(int i=0;i<_rows;i++){      
        for(int j=0; j<_cols_array;j++) {      
          _bits[i][j] = value ? B11111111 : B00000000;     
        }       
      }  
    }
    
    int read(unsigned int row, unsigned int column){
      byte b = _bits[row][ column/8 + (column%8 ? 1 : 0) ];
      unsigned int bit = column%8;
      
      return (b & (1 << bit)) != 0;
    }
    
    void toggle(unsigned int row, unsigned int column){ write(row, column, !read(row, column)); }
    void toggle(){ for(int i=0;i<_rows;i++){      for(int j=0; j<_columns;j++) {      toggle(i,j);   }   }   }
};

/*************************************************************************************************************
*******************************FIM BIT ARRAY *****************************************************************
**************************************************************************************************************/

/*************************************************************************************************************
************************************CLASSE UNIQUE RANDOM******************************************************
**************************************************************************************************************
mais informações aqui: http://fabianoallex.blogspot.com.br/2015/09/arduino-numeros-aleatorio-repetidos-e.html
*************************************************************************************************************/
 
class UniqueRandom{
  private:
    int _index;
    int _min;
    int _max;
    int _size;
    int* _list;
    void _init(int min, int max) {
      _list = 0; 
      if (min < max) { _min = min; _max = max; } else { _min = max; _max = min; }
      _size = _max - _min; 
      _index = 0;
    }    
  public:
    UniqueRandom(int max)           { _init(0,   max); randomize(); } //construtor com 1 parametro
    UniqueRandom(int min, int max)  { _init(min, max); randomize(); } //construtor com 2 parametros
     
    void randomize() {
      _index = 0;
      
      if (_list == 0) { _list = (int*) malloc(size() * sizeof(int)); }  
      for (int i=0; i<size(); i++) {   _list[i] = _min+i;  }   //preenche a lista do menor ao maior valor
       
      //embaralha a lista
      for (int i=0; i<size(); i++) {  
        int r = random(0, size());     //sorteia uma posição qualquer
        int aux = _list[i];               
        _list[i] = _list[r];
        _list[r] = aux;
      }
    }
     
    int next() {                                  //retorna o proximo numero da lista
      int n = _list[_index++];
      if (_index >= size() ) { _index = 0;} //após recuper o ultimo numero, recomeça na posicao 0
      return n;
    }
     
    int size() { return _size; }
     
    ~UniqueRandom(){ free ( _list ); }  //destrutor
};
/*************************************************************************************************************
************************************FIM CLASSE UNIQUE RANDOM**************************************************
*************************************************************************************************************/


/*************************************************************************************************************
*******************************DISPLAY************************************************************************
**************************************************************************************************************/

/*
rotation indica qual parte do display estará para cima

         TOP
L  . . . . . . . . R
E  . . . . . . . . I
F  . . . . . . . . G
T  . . . . . . . . H
   . . . . . . . . T
   . . . . . . . .
   . . . . . . . .
   . . . . . . . .   
       BOTTOM
    
*/
enum Rotation {TOP, LEFT, BOTTOM, RIGHT};

struct Position {
  int lin;
  int col;
};

struct Display {
  int index;
  Position position; 
  Rotation rotation;
};

/*************************************************************************************************************
*******************************FIM DISPLAY********************************************************************
**************************************************************************************************************/

/*************************************************************************************************************
*******************************CLASSE SNAKE GAME**************************************************************
**************************************************************************************************************/

const int SNAKE_MAX_LEN   = 50;  //tamanho maximo da cobra
const int SNAKE_TIME_INIT = 500; //tempo entre deslocamento da cobra (velocidade)
const int SNAKE_TIME_INC  = 15;  //incremento da velocidade

enum Direction { DIR_STOP, DIR_TOP, DIR_LEFT, DIR_BOTTOM, DIR_RIGHT};
enum SnakeStatus { SNAKE_GAME_ON, SNAKE_GAME_OVER };

class SnakeGame{
  private:
    BitArray2D * _display;
    Position _snake_positions[SNAKE_MAX_LEN];
    Position _apple;
    int _length;
    Direction _direction;
    unsigned long _last_millis;
    int _time;
    int _score;
    SnakeStatus _snakeStatus;
    
    UniqueRandom * _ur;  //utilizado no game over
    
    void _generateApple() {
      int lin, col;
      boolean random_ok = false;
      
      while (!random_ok) {
        random_ok = true;
        lin = random(0, _display->rows()-1);
        col = random(0, _display->columns()-1);
        
        for (int p=0; p<_length; p++){
          if (_snake_positions[p].col==col && _snake_positions[p].lin==lin){ //verifica se gerou em um local que não seja a cobra
            random_ok = false;
            break;
          }
        }
      }
      _apple.lin = lin;
      _apple.col = col;
    }
    
    void _gameOver(){ 
      _snakeStatus = SNAKE_GAME_OVER; 
      _direction   = DIR_STOP;
      _time = 20;
    }
    
    void _inc_length(){
      _length++; _score++;
      _time -= SNAKE_TIME_INC;
    }
    
    void _runGameOver(){
      int r = _ur->next();
      int lin = (r / _display->columns());
      int col = (r % _display->columns());
      
      _display->write(lin, col, HIGH );
      
      if ( r>=(_ur->size()-1) || _direction != DIR_STOP ) {  
        _ur->randomize();
        start(); 
      }
    }
    
    void _run(){
      for (int i=_length-1; i>0; i--){
        _snake_positions[i].lin = _snake_positions[i-1].lin;
        _snake_positions[i].col = _snake_positions[i-1].col;
      }
      
      if (_direction == DIR_TOP )    { _snake_positions[0].lin--;  }
      if (_direction == DIR_BOTTOM ) { _snake_positions[0].lin++;  }
      if (_direction == DIR_LEFT )   { _snake_positions[0].col--;  }
      if (_direction == DIR_RIGHT )  { _snake_positions[0].col++;  }
      
      //verifica se ultrapassou o limite do display
      if (_snake_positions[0].lin < 0)                     { _gameOver(); }
      if (_snake_positions[0].lin >= _display->rows() )    { _gameOver(); }
      if (_snake_positions[0].col < 0)                     { _gameOver(); }
      if (_snake_positions[0].col >= _display->columns() ) { _gameOver(); }
      
      //verifica se colidiu na cobra
      for (int i=_length-1; i>0; i--){
        if (_snake_positions[i].lin == _snake_positions[0].lin && _snake_positions[i].col == _snake_positions[0].col) {
          _gameOver();
        }  
      }
      
      //verifica se comeu a maça
      if (_snake_positions[0].col == _apple.col && _snake_positions[0].lin == _apple.lin){
        _inc_length();
        
        if (_length > SNAKE_MAX_LEN) { _length = SNAKE_MAX_LEN; } else {
          _snake_positions[_length-1].lin = _snake_positions[_length-2].lin;
          _snake_positions[_length-1].col = _snake_positions[_length-2].col;
        }
        _generateApple();
      }
      
      //update display
      for (int lin=0; lin<_display->rows(); lin++) {
        for (int col=0; col<_display->columns(); col++) {
          for (int p=0; p<_length; p++){
            boolean val = _snake_positions[p].col==col && _snake_positions[p].lin==lin;
            _display->write( lin, col,  val );
            if (val) {break;}
          }
        }
      }
      _display->write(_apple.lin, _apple.col, HIGH);
      //--
    }
    
  public:
    SnakeGame(BitArray2D * display){ 
      _display = display;
      _ur = new UniqueRandom( _display->rows() * _display->columns() );
      start();
    }
    
    void start(){
      _length = 1;
      _score  = 0;
      _time = SNAKE_TIME_INIT;
      _last_millis = 0;
      _snake_positions[0].lin = _display->rows() / 2;
      _snake_positions[0].col = _display->columns() / 2;
      _direction = DIR_STOP;
      
      _snakeStatus = SNAKE_GAME_ON;
      
      _generateApple();
    }
    
    void left()   { if (_direction == DIR_RIGHT)  return; _direction = DIR_LEFT;   }
    void right()  { if (_direction == DIR_LEFT)   return; _direction = DIR_RIGHT;  }
    void top()    { if (_direction == DIR_BOTTOM) return; _direction = DIR_TOP;    }
    void bottom() { if (_direction == DIR_TOP)    return; _direction = DIR_BOTTOM; }
    
    int getScore(){ return _score; }
    
    int update(){
      int r = false;
      
      if (millis() - _last_millis > _time) {
        r = true;
        _last_millis = millis();
      
        if (_snakeStatus == SNAKE_GAME_ON)   { _run();         }
        if (_snakeStatus == SNAKE_GAME_OVER) { _runGameOver(); }
      }
      
      return r; //r-->indica se houve mudança no display
    }
};

/*************************************************************************************************************
*******************************FIM CLASSE SNAKE GAME**********************************************************
**************************************************************************************************************/



const int LINHAS  = 16;
const int COLUNAS = 8;


/*
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
 * * * * * * * *
*/

Display displays_8x8[] = {
  {0, {0,0}, TOP},
  {1, {8,0}, BOTTOM}
};

BitArray2D ba(LINHAS, COLUNAS); //8 linhas e 20 colunas
SnakeGame snake(&ba);
 
const int PIN_CLOCK   = 5;
const int PIN_DATA    = 7;
const int PIN_LOAD    = 6;
const int QTD_MAX7219 = 2;
 
LedControl lc = LedControl(PIN_DATA,PIN_CLOCK,PIN_LOAD, QTD_MAX7219);

void update_displays_8x8() {
  lc.startWrite();
  
  for (int lin=0; lin<ba.rows(); lin++) {
    for (int col=0; col<ba.columns(); col++) {
      for (int i=0; i<sizeof(displays_8x8)/sizeof(Display); i++) {
        int l = lin - displays_8x8[i].position.lin;
        int c = col - displays_8x8[i].position.col;
        
        if (l>=0 && l<=7 && c>=0 && c<=7) {
          
          if (displays_8x8[i].rotation == BOTTOM) {            c=7-c; l=7-l;   }
          if (displays_8x8[i].rotation == LEFT)   { int aux=c; c=l;   l=7-aux; }
          if (displays_8x8[i].rotation == RIGHT)  { int aux=l; l=c;   c=7-aux; }

          lc.setLed(displays_8x8[i].index, l, c, ba.read(lin, col) );
        }
      }
    }
  }
  
  lc.send();
}


void setup() {
  Serial.begin(9600);
  
  lc.shutdown(0,false);
  lc.setIntensity(0,8);
  lc.clearDisplay(0);  
  lc.shutdown(1,false);
  lc.setIntensity(1,8);
  lc.clearDisplay(1);  
  
  randomSeed(analogRead(A0));
}

void loop() {
  int c = Serial.read();
   
  if (c == 97)  { snake.left();   } //a -> 
  if (c == 98)  { snake.right();  } //b -> 
  if (c == 99)  { snake.top();    } //c -> 
  if (c == 100) { snake.bottom(); } //d -> 
  
  if ( snake.update() ) { update_displays_8x8(); }
}