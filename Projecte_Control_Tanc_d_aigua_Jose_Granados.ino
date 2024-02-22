//Llibreries
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2); // Direcció i tamany de la pantalla 16x2

//Variables d'entrada anlògiques
int pote = 0; // Variable Entrada Potenciometré
int sensor = 0; // Variable Entrada del Sensor

    // Variable Condicions Funcionament bomba
    int Bomba = 0;

          // Variables escalades  entrades analògiques
          int valorsensor; // Variable escalada del sensor.
          int valorpote;// Variable escalada potencimetre SetPoint.

              // Variables de falsejat del senyal del sensor 'Histeresi sensor' i calcul error
              int sensorsuma; // Histeresi sensor per la carrega
              int sensorresta; // Histeresi sensor per la descarrega


                // Variable del Setpoint per visualitzar a la pantalla
                
                    // Variables entrades digitals
                    bool Sensorinferior;
                    bool Sensorsuperior;
                    bool P_Start;
                    bool P_Stop;
                    bool P_Reset;
                    bool display1;
                    bool display2;
    
                            // Variables de funcionament
                    bool enclavament;
                    bool estat;
                    bool enclavament2;
                    bool estat2;
                    bool Activat;
                    bool Aturat;
                    bool Buidar;
                    bool inRange;
                    bool outRange;
       
                          // Variables de funcionament sensor ultrasons
                          
                               // Pins Entrada Ultrasons Trig i Echo
                               const int PinTrig=12;
                               const int PinEcho=11;
                                 
                              // Constant velocitat del sò a temperatura ambient.
                               const float VelSon = 34000.0;
                                 
                              // Numero de lectures del ultrasons
                               const int numLecturas = 15; // Com menys lectures mes precisió pero al tratarse de liquids obtindrem moltes fluctuacions per el moviment del mateix.
                                 
                              // Constants de distancia als 1000 ml i en buit
                               const float distancia1000 = 7.61; // Distancia als 100ml 3,69cm
                               const float distanciaVacio = 19.11; // Distancia amb remanent d'aigua 16,85cm sense res d'aigua 19,11cm
                               float lecturas[numLecturas]; // Vector per guardar les lectures del ultrasons
                               int lecturaActual = 0; // Lectura actual
                               float total = 0; // Total lectures
                               float media = 0; // Mitja de les lectures
                               bool primeraMedia = false; // Variable booleana per la mitja de les lectures per avisar cuan s'ha fet la primera mitja.
                               float quantitatLiquid;// Quantitat de liquid en ml
                               float distanciaLleno; // Distancia omplert en cm
                         
                         

void setup() { // put your setup code here, to run once:
 // Sortides Entrades Digitals
  pinMode (2, INPUT); // Sensor tanc bomba inferior
  pinMode (3, INPUT); // Sensor tanc bomba superior
  pinMode (4, INPUT); // Start
  pinMode (5, INPUT); // Stop
  pinMode (6, INPUT); // Reset
  pinMode (7, INPUT); // Polsador Display 1
  pinMode (8, INPUT); // Pulsador Display 2
  pinMode (10, OUTPUT); // Valvula Carrega
  pinMode (9, OUTPUT); // Valvula descarrega
  pinMode (13, OUTPUT); // Bomba

  // Sensor Ultrasons
      // Declarem com a sortida i entrada del sensor d'ultrasons Trig i Echo
      pinMode(PinTrig, OUTPUT); // Pin 12
      pinMode(PinEcho, INPUT); // Pin 11
     
      // Iniciem el vector de lectures
      for (int i = 0; i < numLecturas; i++) // Si I es igual a 0  i es menor a numero de lectures incrementa el vector I
      {
        lecturas[i] = 0; // Tornem a posar la variable a 0
      }
      
// Reset relés a 0
  digitalWrite (9, HIGH); // Valvula de Carrega
    digitalWrite (10, HIGH); // Valvula descarrega
    digitalWrite (13, HIGH); // Bomba
    
  // Pantalla Display Text inici
  lcd.init();
  lcd.display(); // Activar Pantalla
  lcd.backlight (); // Activar llum
  lcd.setCursor (0, 0);
  lcd.print  ("Control Nivell");
  lcd.setCursor (0, 1);
  lcd.print  ("Tanc d'aigua");
  delay (5000);
  lcd.clear ();
  lcd.setCursor (0, 0);
  lcd.print  ("Jose Granados");
  lcd.setCursor (0, 1);
  lcd.print  ("GS 1r Ari");
  delay (5000);
  lcd.clear (); // Netejar pantalla

  
  Serial.begin(9600); // Visualització en monitor 
 
 }


void loop() {
  // put your main code here, to run repeatedly:

   // Lectura entrades digitals
        Sensorinferior = digitalRead (2);
        Sensorsuperior = digitalRead (3);
        P_Start = digitalRead (4);
        P_Stop =  digitalRead (5);
        P_Reset = digitalRead (6);
        display1 = digitalRead (7);
        display2 = digitalRead (8);

                                                  
  // Crida al Void de condicions de funcionament sensor ultrasons
  Lectura_ultrasons();
  // Crida al Void de condicions de funcionament sensor resistiu
  Calcul_sensors();

    // Condicions Marxa, Atur, Reset polsadors
                    
                    enclavament=estat;
                    enclavament2=estat2;
                    
                       if (P_Start == HIGH){ // Polsador Start posa a 1 la variable estat i Activat, les demes a 0.
                        estat=1;
                        estat2=0;
                       } 
                     
                       else if (P_Reset == HIGH){ // Polsador Reset posa a 1 la variable buidar i l'estat2 i les demes a 0.
                        estat2=1;
                        estat=0;
                       }
                    
                      
                       else if (P_Stop == LOW){ // Polsador Stop posa a 1 la variable aturat i tots els estats a 0
                        estat=0;
                        estat2=0;
                       }
                    
                        
                     if (estat2==0 && estat==0){ // Sistema Aturat
                        Aturat=1;
                        Buidar=0;
                        Activat=0;
                        Sistema();  // Crida al Void de condicions de funcionament del sistema
                        }
                     else if (estat==1){ // Sistema Activat
                        Activat=1;
                        Buidar=0;
                        Aturat=0;
                        Sistema();  // Crida al Void de condicions de funcionament del sistema
                          }
                     else if (estat2==1){ // Sistema en Reset
                        Buidar=1;
                        Activat=0;
                        Aturat=0;
                        Sistema();  // Crida al Void de condicions de funcionament del sistema
                         }
                     
                                               
             // Condicions de funcionament de la Bomba Nivell del tanc exterior, sensors digitals
                             
                     if (Sensorinferior == LOW && Sensorsuperior == LOW) {// Si els dos sensors no estan activats no hi ha aigua i possa la variable bomba a 0
                                Bomba = 0;
                           }   
                         else if (Sensorinferior == HIGH && Sensorsuperior == LOW) {
                                Bomba = 1;
                              }
                              else {
                                Bomba = 1;
                              }  
                                        
    // Visualització en monitor
    
        Serial.print("Bomba");
        Serial.println (  Bomba);
        Serial.print("Histeresi sensor");
        Serial.println(   sensorresta);
        Serial.print(" Activat");
        Serial.println ("");
        Serial.println(Activat);
        Serial.print("ml");
        Serial.println ("");
        Serial.println(quantitatLiquid);
        Serial.print("Histeresi");
        Serial.println (sensorresta);
        Serial.println ("");
        Serial.println ("sensor suma");
        Serial.println ("");
        Serial.println (sensorsuma);
        Serial.print("Sensor Resistiu"); 
        Serial.println ("");
        Serial.println(sensor); 
        Serial.print("Distancia cm");
        Serial.println ("");
        Serial.println(distanciaLleno); 
        Serial.print("inRange");
        Serial.println ("");
        Serial.println(inRange);
        Serial.print("outRange");
        Serial.println ("");
        Serial.println(outRange);
         
           
 
  // Crida al Void de condicions de funcionament de la pantalla
  Pantalla(); // Execució pantalla

  
         
}

    // Void Loop Calcul i lectura sensor
    void Calcul_sensors() {
                   
      sensor = analogRead(A0); // Declaració Entrada analògica Sensor resistiu A0
      valorsensor = map(sensor, 1009, 520, 0, 2000);  // Mapejat del sensor resistiu
          
               // Entrada Potenciometre SetPoint i mapejat dels valors
               pote = analogRead(A1); // Declaració Entrada analògica potenciometre
               valorpote = map(pote, 0, 1021, 0, 2000);  // Mapejat del potenciometre
               
           
                 // Histeresi Sensor Ultrasons
                 sensorsuma = quantitatLiquid + 230; 
                 sensorresta = quantitatLiquid -295;     
                
        } 


       // Void Loop Funcionament Sistema
     void Sistema() {       
                                                 
                                     
          // Condicions de funcionament
                                
               if (Activat == 1) // Comparació entre dos valors si la variable activat es igual a 1 executa el control del tanc d'aigua
                {
                  
                  if (Bomba==1){ // Si els sensor de la bomba detectan que hi ha aigua i per tant la variable Bomba esta a a 1 aquesta permet que s'activi la bomba.
                  digitalWrite (13, LOW); // Bomba 
                  }
                  else{
                  digitalWrite (13, HIGH); // Bomba  // Si el sensor de limit inferior esta aturat no deixa activar la bomba.
                  }
                 
                   if ((sensorresta <= valorpote) && (valorpote <= sensorsuma)) // In range = 1 quan el valor del potenciometre esta entre la resta i la suma del sesnor.
                        {
                          inRange=1;
                          outRange=0;
                        }
        
                   else if ((valorpote < sensorresta) or (valorpote > sensorsuma)) // Out of range = 1 quan el valor del potenciometre esta fora del rang de la resta o la suma del sensor.
                        {
                          inRange=0;
                          outRange=1;
                        }

                
                        if (inRange==1) { // Si l'error esta dintre del rang d'error del nostre sistema atura les valvules de carrega i descarrega.
                        digitalWrite (10, HIGH); // Valvula de Carrega
                        digitalWrite (9, HIGH); // Valvula descarrega
                        
                        }
                       else if (outRange==1) { // Si l'error esta fora del rang del error del nostre sistema...
                         
                           if (valorpote>sensorsuma){ // Si el valor del setpoint es mes gran que la suma del sensor activa la valvula de carrega
                           digitalWrite (10, LOW); // Valvula de Carrega
                           digitalWrite (9, HIGH); // Valvula descarrega
                         
                          }
                              
                          else if (valorpote<sensorresta){ // Si el valor del potenciometre es mes petit que la resta del sensor activa la valvula de descarrega
                          digitalWrite (10, HIGH); // Valvula de Carrega
                          digitalWrite (9, LOW); // Valvula descarrega
                           
                          
                          }
                          else if (quantitatLiquid==valorpote){ // Si el valor que calcula el ultrasons de liquid es igual al valor del potenciometre atura les valvules de carrega i descarrega
                          digitalWrite (10, HIGH); // Valvula de Carrega
                          digitalWrite (9, HIGH); // Valvula descarrega
                           
                            }
                       } 
                    }                                                      
                               
      else if (Aturat == 1) // Si el sistema esta aturat s'aturen les sortides.
            {
          digitalWrite (10, HIGH); // Valvula de Carrega
          digitalWrite (9, HIGH); // Valvula descarrega
          digitalWrite (13, HIGH); // Bomba
           }
                 
      else if (Buidar == 1) // Si el sistema esta en Reset es comença a buidar el tanc obrint la valvula de desccarrega.
            {
            digitalWrite (10, HIGH); // Valvula de Carrega
            digitalWrite (9, LOW); // Valvula descarrega
            digitalWrite (13, HIGH); // Bomba
           
            }
      }

       
  // Void Loop Sensor Ultrasons
          void Lectura_ultrasons() {
          // Eliminem la ultima mesura del vector per tornar a fer un altre lectura
            total = total - lecturas[lecturaActual];
           
            iniciarTrigger(); // Iniciem el void Trigger per anar fent les mesures
           
            //La funció Pulsein obté el temps que tarda en cambiar el estat 'De HIGH a LOW'
            unsigned long tiempo = pulseIn(PinEcho, HIGH);
           
            // Per obtenir la distancia en  cm convertim el temps de microsegons a segons multiplicant per 0.000001 de la formula de la distancia a velocitat del sò
            float distancia = tiempo * 0.000001 * VelSon / 2.0;
           
            // Guardem la distancia al vector de lectura
            lecturas[lecturaActual] = distancia;
           
            // Sumem la lectura a les demes lectures fetes
            total = total + lecturas[lecturaActual];
           
            // Avançem a la següent posició en el vector
            lecturaActual = lecturaActual + 1;
           
            // Una vegada arribem al numero maxim de lectures activem la variable per fer la mitja de les lectures i reiniciem la variable de lectura actual a 0
            if (lecturaActual >= numLecturas)
            {
              primeraMedia = true;
              lecturaActual = 0;
            }
           
            // Calculem la mitja de les lectures guardades.
            media = total / numLecturas;
          
           // Calcul de la lectura del sensor d'ultrasons nomes cuan s'ha calculat la mitja total.
                   if (primeraMedia) // Si la primera mitja de lectures s'ha calculat calcula la distancia que s'ha omplert
                        {
                          distanciaLleno = distanciaVacio - media; // Calcula la distancia omplerta restant la distancia en buit de la mitja de les lectures en real.
                          quantitatLiquid = distanciaLleno * 1000 / distancia1000; // Calcula la cantitat de liqui multiplicant la distancia del liquid per 1000 i ho divideix de la distancia als 1000ml.
                        }     
                              
                  delay(500); // Espera per tornar a mesurar.
   
                 }
                                                    
  // Void que inicia el trigger per començar les lectures.
void iniciarTrigger(){
  // Posem el trigger en LOW i esperem 2 microsegons
  digitalWrite(PinTrig, LOW);
  delayMicroseconds(2);
 
  // Activem el trigger i esperem 10 microsegons a que retorni la ona i no es solapin l'una del altre.
  digitalWrite(PinTrig, HIGH);
  delayMicroseconds(10);
 
  // Tornem a posar el Trig en low per tornar a enviar un altre senyal.
  digitalWrite(PinTrig, LOW);
}                

    // Void Pantalla
    void Pantalla() {
    
      // Condicions visualització pantalla
    
      if (display1 == HIGH && display2 == LOW) { // Sensor Ultrasons
        lcd.clear (); // Netejar pantalla
        lcd.setCursor (0, 1);
        lcd.print  (quantitatLiquid);
        lcd.setCursor (9, 1);
        lcd.print  (valorpote);
        lcd.setCursor (0, 0);
        lcd.print  ("Nivell   SetPoint");
        lcd.display(); // Activar Pantalla
       
      }
      
      else if (display2 == HIGH && display1 == LOW) { // Sensor Resistiu
        lcd.clear (); // Netejar pantalla
        lcd.setCursor (0, 1);
        lcd.print  (valorsensor);
        lcd.setCursor (9, 1);
        lcd.print  (valorpote);
        lcd.setCursor (0, 0);
        lcd.print  ("Nivell  SetPoint");
        lcd.display(); // Activar Pantalla
    
      }
      else { // Menu pantalla
        lcd.clear (); // Netejar pantalla
        lcd.setCursor (0, 1);
        lcd.print  ("Ultrasons");
        lcd.setCursor (0, 0);
        lcd.print  ("Resistiu");
        lcd.display(); // Activar Pantalla
        
      }
    }
