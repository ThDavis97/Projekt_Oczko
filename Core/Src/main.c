/* USER CODE BEGIN Header */
/** \brief
  * @file main.c
  * @brief Główny plik programu, który odpowiada za obsługę zadań w FreeRTOS
  * 	oraz część sprzętową. Zawiera PINOUT układu
  *
  * @authors Dawid Stolarski, Bartosz Czemiel oraz Dawid Lypaczewski
  * @date 15-02-2022
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  * @PINOUT:
  *
  * -----------------------------------Przyciski:
  * PD8  - Przycisk nowa gra;
  * PD9  - Przycisk reset;
  * PD10 - Przycisk wystarczy kart;
  * PD11 - Przycisk kolejna karta;
  * Wszystkie przyciski należy podłączyć do GND;
  * -----------------------------------------LCD:
  * D4 - PE7;
  * D5 - PE8;
  * D6 - PE9;
  * D7 - PE10;
  * RS - PE11;
  * E  - PE12;
  * -----------------------------------------ADC:
  * PA1 - kabelek zbierający zakłocenia do ADC;
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "lcd.h"    //Biblioteka umożliwiająca obsługę ekranu LCD HD44780

/*
 *
 */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
//Definicje dla ustawień systemowych:
#define time_to_press 5 ///<czas wymagany aby STM przyjal sygnal z przycisku
	///<1 odpowiada 20ms, 2 odpowiada 40ms, ...
#define time_to_wait 0///<czas jaki bedzie czekac kolejka na nadanie/odbior danych
#define amount_rounds_to_win_game 3 ///<ilość rund jaką należy wygrać, aby wygrać grę
#define value_fight_CPU 16 ///<Wartość do której włacznie, komputer będzie dobierał karty

//Definicje ułatwiające obsługę ekranu LCD
//Wyswietlanie "specjalnych" ekranow tzw. szablonow
#define LCD_show_template xQueueSend(interfaceLCDqueue, &special_screen, time_to_wait) //</Na ekranie LCD wyświetl specjalny szablon
#define screen_default 1 //"U:         |    "
					     //"C:         |    "
#define screen_loading 2 //"   Shuffling    "
					     //"     the cards  "
#define screen_winner_this_round_U 3 //              "W"  W - Winner - wygrany
					     	 	     //              "L"  L - Loser - przegrany
#define screen_winner_this_round_C 4 //              "L"
					     	 	     //              "W"
#define screen_winner_this_round_D 5 //              "D"  D - Draw - remis
					     	 	     //              "D"
#define screen_clean_place_cards 6   //Wyczyść ekran z wylosowanych kart oraz liter W,L i D

//Wyswietlenie ekranu zwyciezcy
#define screen_winner_this_game_U 1  //"USER won theGame"
                                     //"  USER X:Y CPU  "
#define screen_winner_this_game_C 2  //"CPU won the game"
                                     //"  USER X:Y CPU  "

//Definicje ułatwiające obsługę kolejek:
#define LCD_show_points_U xQueueSend(points_U_LCDqueue, &number_of_points_USER, time_to_wait) ///<Wyświetl liczbę punktów gracza na ekranie LCD
#define LCD_show_points_C xQueueSend(points_C_LCDqueue, &number_of_points_CPU, time_to_wait)  ///<Wyświetl liczbę punktów CPU na ekranie LCD
#define LCD_show_round_wins_U xQueueSend(wins_U_LCDqueue, &number_of_wins_USER, time_to_wait) ///<Wyświetl liczbę wygranych rund gracza na ekranie LCD
#define LCD_show_round_wins_C xQueueSend(wins_C_LCDqueue, &number_of_wins_CPU, time_to_wait)  ///<Wyświetl liczbę wygranych rund CPU na ekranie LCD

//Wyswietlenie wylosowanych kart na ekranie LCD
//przez gracza
#define LCD_show_card1_U xQueueSend(card1_U_LCDqueue, &card_to_show_U[1], time_to_wait) ///<Wyświetlenie wylosowanej karty nr 1 przez gracza na ekranie LCD
#define LCD_show_card2_U xQueueSend(card2_U_LCDqueue, &card_to_show_U[2], time_to_wait) ///<Wyświetlenie wylosowanej karty nr 2 przez gracza na ekranie LCD
#define LCD_show_card3_U xQueueSend(card3_U_LCDqueue, &card_to_show_U[3], time_to_wait) ///<Wyświetlenie wylosowanej karty nr 3 przez gracza na ekranie LCD
#define LCD_show_card4_U xQueueSend(card4_U_LCDqueue, &card_to_show_U[4], time_to_wait) ///<Wyświetlenie wylosowanej karty nr 4 przez gracza na ekranie LCD
#define LCD_show_card5_U xQueueSend(card5_U_LCDqueue, &card_to_show_U[5], time_to_wait) ///<Wyświetlenie wylosowanej karty nr 5 przez gracza na ekranie LCD
#define LCD_show_card6_U xQueueSend(card6_U_LCDqueue, &card_to_show_U[6], time_to_wait) ///<Wyświetlenie wylosowanej karty nr 6 przez gracza na ekranie LCD
#define LCD_show_card7_U xQueueSend(card7_U_LCDqueue, &card_to_show_U[7], time_to_wait) ///<Wyświetlenie wylosowanej karty nr 7 przez gracza na ekranie LCD
#define LCD_show_card8_U xQueueSend(card8_U_LCDqueue, &card_to_show_U[8], time_to_wait) ///<Wyświetlenie wylosowanej karty nr 8 przez gracza na ekranie LCD
#define LCD_show_card9_U xQueueSend(card9_U_LCDqueue, &card_to_show_U[9], time_to_wait) ///<Wyświetlenie wylosowanej karty nr 9 przez gracza na ekranie LCD
//przez komputer
#define LCD_show_card1_C xQueueSend(card1_C_LCDqueue, &card_to_show_C[1], time_to_wait); ///<Wyświetlenie wylosowanej karty nr 1 przez komputer na ekranie LCD
#define LCD_show_card2_C xQueueSend(card2_C_LCDqueue, &card_to_show_C[2], time_to_wait); ///<Wyświetlenie wylosowanej karty nr 2 przez komputer na ekranie LCD
#define LCD_show_card3_C xQueueSend(card3_C_LCDqueue, &card_to_show_C[3], time_to_wait); ///<Wyświetlenie wylosowanej karty nr 3 przez komputer na ekranie LCD
#define LCD_show_card4_C xQueueSend(card4_C_LCDqueue, &card_to_show_C[4], time_to_wait); ///<Wyświetlenie wylosowanej karty nr 4 przez komputer na ekranie LCD
#define LCD_show_card5_C xQueueSend(card5_C_LCDqueue, &card_to_show_C[5], time_to_wait); ///<Wyświetlenie wylosowanej karty nr 5 przez komputer na ekranie LCD
#define LCD_show_card6_C xQueueSend(card6_C_LCDqueue, &card_to_show_C[6], time_to_wait); ///<Wyświetlenie wylosowanej karty nr 6 przez komputer na ekranie LCD
#define LCD_show_card7_C xQueueSend(card7_C_LCDqueue, &card_to_show_C[7], time_to_wait); ///<Wyświetlenie wylosowanej karty nr 7 przez komputer na ekranie LCD
#define LCD_show_card8_C xQueueSend(card8_C_LCDqueue, &card_to_show_C[8], time_to_wait); ///<Wyświetlenie wylosowanej karty nr 8 przez komputer na ekranie LCD
#define LCD_show_card9_C xQueueSend(card9_C_LCDqueue, &card_to_show_C[9], time_to_wait); ///<Wyświetlenie wylosowanej karty nr 9 przez komputer na ekranie LCD


uint16_t ADC_measure; ///<Zmienna przechowujaca wyniki pomiaru z przetwornika ADC

uint8_t random_card_board_1[52]; ///<Tablica przechowujaca wylosowane karty (cala talie)
uint8_t nr_random_card_to_draw_board_1 = 0; ///<Numer tablicy gdzie umiescic wylosowana karte

//Liczba kart do wylosowania
uint8_t number_of_cards_2 = 4;
uint8_t number_of_cards_3 = 4;
uint8_t number_of_cards_4 = 4;
uint8_t number_of_cards_5 = 4;
uint8_t number_of_cards_6 = 4;
uint8_t number_of_cards_7 = 4;
uint8_t number_of_cards_8 = 4;
uint8_t number_of_cards_9 = 4;
uint8_t number_of_cards_T = 4;
uint8_t number_of_cards_J = 4;
uint8_t number_of_cards_Q = 4;
uint8_t number_of_cards_K = 4;
uint8_t number_of_cards_A = 4;
uint8_t assignment_card_to_board = 0; ///<umożliwia odpowiednie przypisanie wylosowanych kart do tablicy
uint8_t if_board_1_prepare = 0; ///<Czy tablica 1 z wylosowanymi kartami jest gotowa? 1 - tak, 0 - nie

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

TIM_HandleTypeDef htim4;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */
//Funkcja do "przeliczania" kart na liczby np. 13(K - King) -> 4
  uint8_t card_for_number (uint8_t card)
  {
	  if (card <= 10) return card;
	  else
	  {
		  switch (card)
		  	  {
		  	  case 11: card = 2; break;  //Gdy pojawi sie Jopek
		  	  case 12: card = 3; break;  //Gdy pojawi sie Dama
		  	  case 13: card = 4; break;  //Gdy pojawi sie Krol
		  	  case 14: card = 11; break; //Gdy pojawi sie As
		  	  }
		  	  return card;
	  }
  }

// Zmienne przechowujace czas wcisniecia przycisku
//(ma to na celu zrobienie debouncera)
// 1 w zmiennej odpowiada 20 ms, 5 - 100ms ...
uint8_t DebCount_nowa_gra = 0; ///<Zmienna przechowująca czas wciśnięcia przycisku nowa gra. 1 odpowiada 20ms
uint8_t DebCount_reset = 0;    ///<Zmienna przechowująca czas wciśnięcia przycisku reset. 1 odpowiada 20ms
uint8_t DebCount_wystarczy_kart = 0; ///<Zmienna przechowująca czas wciśnięcia przycisku wystarczy kart. 1 odpowiada 20ms
uint8_t DebCount_kolejna_karta = 0;  ///<Zmienna przechowująca czas wciśnięcia przycisku kolejna karta. 1 odpowiada 20ms

//zmienne do obslugi debouncingu (do przelaczenia jednorazowo stanu - toggle)
uint8_t Switched_nowa_gra = 0; ///<zmienna do obslugi debouncingu (do przelaczenia jednorazowo stanu - toggle)
uint8_t Switched_reset = 0; ///<zmienna do obslugi debouncingu (do przelaczenia jednorazowo stanu - toggle)
uint8_t Switched_wystarczy_kart = 0; ///<zmienna do obslugi debouncingu (do przelaczenia jednorazowo stanu - toggle)
uint8_t Switched_kolejna_karta = 0; ///<zmienna do obslugi debouncingu (do przelaczenia jednorazowo stanu - toggle)

//Zmienne informujące że przycisk (po debouncerze) został wciśnięty
//Dzięki temu wiadomo, że trzeba aktywować odpowiedni semafor
uint8_t Pressed_nowa_gra = 0; ///<Zmienne informujące że przycisk (po debouncerze) został wciśnięty. Dzięki temu wiadomo, że trzeba aktywować odpowiedni semafor
uint8_t Pressed_reset = 0; ///<Zmienne informujące że przycisk (po debouncerze) został wciśnięty. Dzięki temu wiadomo, że trzeba aktywować odpowiedni semafor
uint8_t Pressed_wystarczy_kart = 0; ///<Zmienne informujące że przycisk (po debouncerze) został wciśnięty. Dzięki temu wiadomo, że trzeba aktywować odpowiedni semafor
uint8_t Pressed_kolejna_karta = 0; ///<Zmienne informujące że przycisk (po debouncerze) został wciśnięty. Dzięki temu wiadomo, że trzeba aktywować odpowiedni semafor

//Obsluga FreeRTOS
//Handlery dla zadan:
xTaskHandle Task1_Handler; ///<Handler dla Task1 (obsługa LCD)
xTaskHandle Task2_Handler; ///<Handler dla Task2 (obsługa przycisków)
xTaskHandle Task3_Handler; ///<Handler dla Task3 (obsługa całej gry)

//Funckcje dla zadan
void Task1 (void *argument); ///<Funkcja dla Task1 (obsługa LCD)
void Task2 (void *argument); ///<Funkcja dla Task2 (obsługa przycisków)
void Task3 (void *argument); ///<Funkcja dla Task3 (obsługa całej gry)

//ID dla zadan
osThreadId Task1Handle; ///<ID dla Task1 (obsługa LCD)
osThreadId Task2Handle; ///<ID dla Task2 (obsługa przycisków)
osThreadId Task3Handle; ///<ID dla Task3 (obsługa całej gry)

//Handlery dla kolejek:
xQueueHandle interfaceLCDqueue; ///<Do przesyłania tzw. szablonów które mają zostać wyświetlone na LCD
xQueueHandle wins_U_LCDqueue;   ///<Do przesyłania liczby wygranych rund przez gracza które mają zostać wyświetlone na LCD poczas gry
xQueueHandle wins_C_LCDqueue;   ///<Do przesyłania liczby wygranych rund przez komputer które mają zostać wyświetlone na LCD poczas gry
xQueueHandle points_U_LCDqueue; ///<Do przesyłania liczby zdobytych punktów przez gracza które mają zostać wyświetlone na LCD przy podsumowaniu gry
xQueueHandle points_C_LCDqueue; ///<Do przesyłania liczby zdobytych punktów przez komputer które mają zostać wyświetlone na LCD przy podsumowaniu gry

//Do wyświetlenia wynikow na koncu
xQueueHandle winner_the_gameLCDqueue;
xQueueHandle wins_U_end_LCDqueue; ///<Do przesyłania liczby wygranych rund przez gracza które mają zostać wyświetlone na LCD przy podsumowaniu gry
xQueueHandle wins_C_end_LCDqueue; ///<Do przesyłania liczby wygranych rund przez komputer które mają zostać wyświetlone na LCD przy podsumowaniu gry

xQueueHandle card1_U_LCDqueue; ///<Do przesyłania wylosowanej karty nr 1 przez gracza
xQueueHandle card2_U_LCDqueue; ///<Do przesyłania wylosowanej karty nr 2 przez gracza
xQueueHandle card3_U_LCDqueue; ///<Do przesyłania wylosowanej karty nr 3 przez gracza
xQueueHandle card4_U_LCDqueue; ///<Do przesyłania wylosowanej karty nr 4 przez gracza
xQueueHandle card5_U_LCDqueue; ///<Do przesyłania wylosowanej karty nr 5 przez gracza
xQueueHandle card6_U_LCDqueue; ///<Do przesyłania wylosowanej karty nr 6 przez gracza
xQueueHandle card7_U_LCDqueue; ///<Do przesyłania wylosowanej karty nr 7 przez gracza
xQueueHandle card8_U_LCDqueue; ///<Do przesyłania wylosowanej karty nr 8 przez gracza
xQueueHandle card9_U_LCDqueue; ///<Do przesyłania wylosowanej karty nr 9 przez gracza

xQueueHandle card1_C_LCDqueue; ///<Do przesyłania wylosowanej karty nr 1 przez komputer
xQueueHandle card2_C_LCDqueue; ///<Do przesyłania wylosowanej karty nr 2 przez komputer
xQueueHandle card3_C_LCDqueue; ///<Do przesyłania wylosowanej karty nr 3 przez komputer
xQueueHandle card4_C_LCDqueue; ///<Do przesyłania wylosowanej karty nr 4 przez komputer
xQueueHandle card5_C_LCDqueue; ///<Do przesyłania wylosowanej karty nr 5 przez komputer
xQueueHandle card6_C_LCDqueue; ///<Do przesyłania wylosowanej karty nr 6 przez komputer
xQueueHandle card7_C_LCDqueue; ///<Do przesyłania wylosowanej karty nr 7 przez komputer
xQueueHandle card8_C_LCDqueue; ///<Do przesyłania wylosowanej karty nr 8 przez komputer
xQueueHandle card9_C_LCDqueue; ///<Do przesyłania wylosowanej karty nr 9 przez komputer

//Semafory
//Handlery:
xSemaphoreHandle Semaphore_nowa_gra_pressed; 		///<Semafor informujący czy przycisk nowa gra został wciśnięty
xSemaphoreHandle Semaphore_reset_pressed; 			///<Semafor informujący czy przycisk reset został wciśnięty
xSemaphoreHandle Semaphore_kolejna_karta_pressed;	///<Semafor informujący czy przycisk kolejna karta został wciśnięty
xSemaphoreHandle Semaphore_wystarczy_kart_pressed;	///<Semafor informujący czy przycisk wystarczy kart został wciśnięty

//ID
osSemaphoreId Semaphore_nowa_gra_pressed_handle;		///<Semafor informujący czy przycisk nowa gra został wciśnięty
osSemaphoreId Semaphore_reset_pressed_handle;			///<Semafor informujący czy przycisk reset został wciśnięty
osSemaphoreId Semaphore_kolejna_karta_pressed_handle;	///<Semafor informujący czy przycisk kolejna karta został wciśnięty
osSemaphoreId Semaphore_wystarczy_kart_pressed_handle;	///<Semafor informujący czy przycisk wystarczy kart został wciśnięty


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM4_Init(void);
static void MX_ADC1_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */


/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */


  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */

    //OBSLUGA RZECZY ZA POMOCA HARDWARE-----------------------------------------------------------
   	//Włączenie odpowiednich peryferiów:
   	HAL_TIM_Base_Start_IT(&htim4); //Włączenie przewań od TIM4
   	HAL_ADC_Start(&hadc1); //Włączenie obsługi przetwornika ADC na kanale 1

   	//OBSLUGA FreeRtos----------------------------------------------------------------------------
   	//Utworzenie zadań
   	xTaskCreate(Task1, "LCD_support", 128, NULL, 0, &Task1_Handler);
   	xTaskCreate(Task2, "Button_support", 128, NULL, 0, &Task2_Handler);
   	xTaskCreate(Task3, "Game_management", 128, NULL, 0, &Task3_Handler);

   	//Utworzenie kolejek
   	interfaceLCDqueue = xQueueCreate(10, sizeof(uint8_t)); //do wyświetlania "szablonów" na ekranie LCD
   	wins_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));   //do wyświetlania liczby wygranych rund przez gracza na ekranie LCD
   	wins_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));   //do wyświetlania liczby wygranych rund przez komputer na ekranie LCD
   	points_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t)); //do wyświetlania liczby zdobytych punktów przez gracza na ekranie LCD
   	points_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t)); //do wyświetlania liczby zdobytych punktów przez komputer na ekranie LCD

   	wins_U_end_LCDqueue = xQueueCreate(10, sizeof(uint8_t)); //do wyświetlenia liczby wygranych rund przez gracza na ekranie podsumowywującym grę
	wins_C_end_LCDqueue = xQueueCreate(10, sizeof(uint8_t)); //do wyświetlenia liczby wygranych rund przez komputer na ekranie podsumowywującym grę
	winner_the_gameLCDqueue = xQueueCreate(10, sizeof(uint8_t)); //do wyświetlenia kto wygrał grę

	//Kolejki służące do przesłania informacji jaką wylosowaną kartę wyświetlić na ekranie LCD
	//Karty dla gracza
   	card1_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card2_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card3_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card4_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card5_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card6_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card7_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card8_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card9_U_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	//Karty dla komputera (CPU)
   	card1_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card2_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card3_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card4_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card5_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card6_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card7_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card8_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));
   	card9_C_LCDqueue = xQueueCreate(10, sizeof(uint8_t));

   	//Utworzenie semaforow binarnych
   	//Do zarządzania przyciskami
   	vSemaphoreCreateBinary(Semaphore_nowa_gra_pressed); //informuje czy przycisk nowa gra został wciśnięty
   	vSemaphoreCreateBinary(Semaphore_reset_pressed); //informuje czy przycisk reset został wciśnięty
   	vSemaphoreCreateBinary(Semaphore_kolejna_karta_pressed); //informuje czy przycisk kolejna karta został wciśnięty
   	vSemaphoreCreateBinary(Semaphore_wystarczy_kart_pressed);//informuje czy przycisk wystarczy kart został wciśnięty

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


  while (1)
  {
	HAL_Delay(1000);
	HAL_GPIO_TogglePin(LD3_GPIO_Port, LD3_Pin);

	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
	{
		ADC_measure = HAL_ADC_GetValue(&hadc1);
		HAL_ADC_Start(&hadc1);
	}

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */
  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief TIM4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM4_Init(void)
{

  /* USER CODE BEGIN TIM4_Init 0 */

  /* USER CODE END TIM4_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM4_Init 1 */

  /* USER CODE END TIM4_Init 1 */
  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 50000-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 40;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim4, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM4_Init 2 */

  /* USER CODE END TIM4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CS_I2C_SPI_Pin|D4_Pin|D5_Pin|D6_Pin
                          |D7_Pin|RS_Pin|E_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(OTG_FS_PowerSwitchOn_GPIO_Port, OTG_FS_PowerSwitchOn_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : CS_I2C_SPI_Pin D4_Pin D5_Pin D6_Pin
                           D7_Pin RS_Pin E_Pin */
  GPIO_InitStruct.Pin = CS_I2C_SPI_Pin|D4_Pin|D5_Pin|D6_Pin
                          |D7_Pin|RS_Pin|E_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PE4 PE5 MEMS_INT2_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|MEMS_INT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = OTG_FS_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(OTG_FS_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PDM_OUT_Pin */
  GPIO_InitStruct.Pin = PDM_OUT_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(PDM_OUT_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_EVT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : I2S3_WS_Pin */
  GPIO_InitStruct.Pin = I2S3_WS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(I2S3_WS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SPI1_SCK_Pin SPI1_MISO_Pin SPI1_MOSI_Pin */
  GPIO_InitStruct.Pin = SPI1_SCK_Pin|SPI1_MISO_Pin|SPI1_MOSI_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : CLK_IN_Pin PB12 */
  GPIO_InitStruct.Pin = CLK_IN_Pin|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : Przycisk_nowa_gra_Pin Przycisk_reset_Pin Przycisk_wystarczy_kart_Pin Przycisk_kolejna_karta_Pin */
  GPIO_InitStruct.Pin = Przycisk_nowa_gra_Pin|Przycisk_reset_Pin|Przycisk_wystarczy_kart_Pin|Przycisk_kolejna_karta_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : LD4_Pin LD3_Pin LD5_Pin LD6_Pin
                           Audio_RST_Pin */
  GPIO_InitStruct.Pin = LD4_Pin|LD3_Pin|LD5_Pin|LD6_Pin
                          |Audio_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : I2S3_MCK_Pin I2S3_SCK_Pin I2S3_SD_Pin */
  GPIO_InitStruct.Pin = I2S3_MCK_Pin|I2S3_SCK_Pin|I2S3_SD_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : VBUS_FS_Pin */
  GPIO_InitStruct.Pin = VBUS_FS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(VBUS_FS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : OTG_FS_ID_Pin OTG_FS_DM_Pin OTG_FS_DP_Pin */
  GPIO_InitStruct.Pin = OTG_FS_ID_Pin|OTG_FS_DM_Pin|OTG_FS_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : OTG_FS_OverCurrent_Pin */
  GPIO_InitStruct.Pin = OTG_FS_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(OTG_FS_OverCurrent_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

//Obsługa LCD
void Task1 (void *argument)
{
	//Definicje do obsługi ekranu LCD
	Lcd_PortType ports[] = { D4_GPIO_Port, D5_GPIO_Port, D6_GPIO_Port,D7_GPIO_Port };
	Lcd_PinType pins[] = { D4_Pin, D5_Pin, D6_Pin, D7_Pin };
	Lcd_HandleTypeDef lcd = Lcd_create(ports, pins, RS_GPIO_Port, RS_Pin, E_GPIO_Port, E_Pin, LCD_4_BIT_MODE);
	Lcd_init(&lcd);
	Lcd_clear(&lcd);

	uint8_t data_received; //Zmienna przechowujaca otrzymane dane

	//Wyświetlenie ekranu startowego
	Lcd_cursor(&lcd,0,0); //Ustawienie kursora na zerowej pozycji zerowego wiersza
	Lcd_string(&lcd," Press new game "); //Wyświetlenie napisu
	Lcd_cursor(&lcd,1,0);
	Lcd_string(&lcd,"BestProjectOn SW");

	for(;;)
	{
		//Wyswietlanie szablonow na ekranie LCD
		if((xQueueReceive(interfaceLCDqueue, &(data_received), 10)) == pdTRUE)
		{
			if (data_received == screen_default) { Lcd_cursor(&lcd,0,0); Lcd_string(&lcd,"U:         |    ");
												   Lcd_cursor(&lcd,1,0); Lcd_string(&lcd,"C:         |    "); }
			if (data_received == screen_loading) { Lcd_cursor(&lcd,0,2); Lcd_string(&lcd,"Shuffling");
			                                       Lcd_cursor(&lcd,0,12); Lcd_string(&lcd,"   ");
												   Lcd_cursor(&lcd,1,2); Lcd_string(&lcd,"the cards");
												   Lcd_cursor(&lcd,1,12); Lcd_string(&lcd,"   ");}
			if (data_received == screen_winner_this_round_U) { Lcd_cursor(&lcd,0,14); Lcd_string(&lcd,"W");
												   	   	   	   Lcd_cursor(&lcd,1,14); Lcd_string(&lcd,"L"); }
			if (data_received == screen_winner_this_round_C) { Lcd_cursor(&lcd,0,14); Lcd_string(&lcd,"L");
															   Lcd_cursor(&lcd,1,14); Lcd_string(&lcd,"W"); }
			if (data_received == screen_winner_this_round_D) { Lcd_cursor(&lcd,0,14); Lcd_string(&lcd,"D");
															   Lcd_cursor(&lcd,1,14); Lcd_string(&lcd,"D"); }
			if (data_received == screen_clean_place_cards) 	 { Lcd_cursor(&lcd,0,2); Lcd_string(&lcd,"         ");
															   Lcd_cursor(&lcd,0,12); Lcd_string(&lcd,"   ");
															   Lcd_cursor(&lcd,1,2); Lcd_string(&lcd,"         ");
															   Lcd_cursor(&lcd,1,12); Lcd_string(&lcd,"   ");}
		}

		//-----------------------------------------------------------------WYGRANE_RUNDY---------------------------
		//Wyswietlenie liczby wygranych rund przez gracza
		if(((xQueueReceive(wins_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 0) && (data_received <= 9))
		{	Lcd_cursor(&lcd,0,15);	Lcd_int(&lcd,data_received); }

		//Wyswietlenie liczby wygranych rund przez komputer
		if(((xQueueReceive(wins_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 0) && (data_received <= 9))
		{	Lcd_cursor(&lcd,1,15); Lcd_int(&lcd,data_received); }

		//----------------------------------------------------------------ZDOBYTE_PUNKTY---------------------------
		//Wyswietlenie liczby punktow zdobytych przez gracza
		if(((xQueueReceive(points_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 0) && (data_received <= 99)) //Tutaj wrzuc 31
		{	Lcd_cursor(&lcd,0,12); Lcd_int(&lcd,data_received); }

		//Wyswietlenie liczby punktow zdobytych przez komputer
		if(((xQueueReceive(points_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 0) && (data_received <= 99)) //Tutaj wrzuc 31
		{	Lcd_cursor(&lcd,1,12); Lcd_int(&lcd,data_received); }



		//--------------------------------------------------------WYSWIETLANIE WYLOSOWANYCH KART-------------------
		//DLA GRACZA
		//Karta nr 1
		if(((xQueueReceive(card1_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,0,2);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 2
		if(((xQueueReceive(card2_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,0,3);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 3
		if(((xQueueReceive(card3_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,0,4);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 4
		if(((xQueueReceive(card4_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,0,5);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 5
		if(((xQueueReceive(card5_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,0,6);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 6
		if(((xQueueReceive(card6_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,0,7);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 7
		if(((xQueueReceive(card7_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,0,8);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 8
		if(((xQueueReceive(card8_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,0,9);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 9
		if(((xQueueReceive(card9_U_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,0,10);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case	 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//WYSWIETLANIE WYLOSOWANYCH KART------------------------------------------------------------------------------------------------------
		//DLA KOMPUTERA
		//Karta nr 1
		if(((xQueueReceive(card1_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
						&& (data_received >= 0) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,1,2);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 2
		if(((xQueueReceive(card2_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,1,3);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 3
		if(((xQueueReceive(card3_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,1,4);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 4
		if(((xQueueReceive(card4_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,1,5);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 5
		if(((xQueueReceive(card5_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,1,6);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 6
		if(((xQueueReceive(card6_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,1,7);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 7
		if(((xQueueReceive(card7_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,1,8);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 8
		if(((xQueueReceive(card8_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,1,9);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Karta nr 9
		if(((xQueueReceive(card9_C_LCDqueue, &(data_received), time_to_wait)) == pdTRUE)
				&& (data_received >= 2) && (data_received <= 14))
		{
			Lcd_cursor(&lcd,1,10);
			if ((data_received >= 2) && (data_received <= 9)) Lcd_int(&lcd,data_received);
			else switch (data_received)
			{
			case 10: { Lcd_string(&lcd,"T"); break; }
			case 11: { Lcd_string(&lcd,"J"); break; }
			case 12: { Lcd_string(&lcd,"Q"); break; }
			case 13: { Lcd_string(&lcd,"K"); break;	}
			case	 14: { Lcd_string(&lcd,"A"); break;	}
			default: { Lcd_string(&lcd,"X"); break;	}
			}//End Switch
		}

		//Wyswietlenie ekranu dla zwyciezcy
		if(((xQueueReceive(winner_the_gameLCDqueue, &(data_received), time_to_wait)) == pdTRUE))
		{
			if (data_received == screen_winner_this_game_U)
			{
				Lcd_cursor(&lcd,0,0); Lcd_string(&lcd,"USER won theGame");
				Lcd_cursor(&lcd,1,0); Lcd_string(&lcd,"  USER  :  CPU  ");
			}
			else if (data_received == screen_winner_this_game_C)
			{
				Lcd_cursor(&lcd,0,0); Lcd_string(&lcd,"CPU won the game");
				Lcd_cursor(&lcd,1,0); Lcd_string(&lcd,"  USER  :  CPU  ");
			}
		}

		//Wyświetlenie wygranych rund przez gracza na ekranie podsumowywującym grę
		if(((xQueueReceive(wins_U_end_LCDqueue, &(data_received), time_to_wait)) == pdTRUE))
		{
			Lcd_cursor(&lcd,1,7);
			Lcd_int(&lcd,data_received);
		}

		//Wyświetlenie wygranych rund przez komputer na ekranie podsumowywującym grę
		if(((xQueueReceive(wins_C_end_LCDqueue, &(data_received), time_to_wait)) == pdTRUE))
		{
			Lcd_cursor(&lcd,1,9);
			Lcd_int(&lcd,data_received);
		}
		osDelay(100);
	}
}

//Obsługa przycisków
void Task2 (void *argument)
{
	//Wyzerowanie semaforow obsługujących przyciski:
	xSemaphoreTake(Semaphore_nowa_gra_pressed, 0);
	xSemaphoreTake(Semaphore_reset_pressed, 0);
	xSemaphoreTake(Semaphore_kolejna_karta_pressed, 0);
	xSemaphoreTake(Semaphore_wystarczy_kart_pressed, 0);

	for(;;)
	{
		if (Pressed_nowa_gra == 1)
		{
			xSemaphoreGive(Semaphore_nowa_gra_pressed);
			Pressed_nowa_gra = 0;
		}
		else if (Pressed_reset == 1)
		{
			xSemaphoreGive(Semaphore_reset_pressed);
			Pressed_reset = 0;
		}
		else if (Pressed_kolejna_karta == 1)
		{
			xSemaphoreGive(Semaphore_kolejna_karta_pressed);
			Pressed_kolejna_karta = 0;
		}
		else if (Pressed_wystarczy_kart == 1)
		{
			xSemaphoreGive(Semaphore_wystarczy_kart_pressed);
			Pressed_wystarczy_kart = 0;
		}

		osDelay(100);
	}
}

//Zarzadzanie grą
void Task3 (void *argument)
{
	//Zdobyte punkty, wygrane rundy
	uint8_t number_of_wins_USER = 0;//liczba wygranych rund przez gracza
	uint8_t number_of_wins_CPU = 0; //liczba wygranych rund przez komputer
	uint8_t number_of_points_USER = 0;//liczba punktow zdobyta w obecnej rundzie przez gracza
	uint8_t number_of_points_CPU = 0; //liczba punktow zdobyta w obecnej rundzie przez komputer

	//Dobieranie kart
	uint8_t nr_cards_drawn_USER = 0; //Liczba dobranych kart przez gracza
	uint8_t nr_cards_drawn_CPU = 0; //Liczba dobranych kart przez komputer
	uint8_t nr_of_cards_taken = 0; //liczba wzietych kart z talii
	uint8_t allow_draw_a_card_USER = 0; //1 - wolno dobierac karty, 0 - nie wolno dobierac kart
	uint8_t allow_draw_a_card_CPU = 0;

	//Rozeznanie czasowe obecnej rundy i całej gry
	uint8_t round_continues = 1; //Czy runda nadal trwa? 1 - Tak, 0 - Nie
	uint8_t if_started_game = 0; //1 gdy gra została rozpoczęta, 0 gdy jeszcze nie została rozpoczęta
	uint8_t game_finish = 0; //1 - gra zostala zakonczona, pojawil sie ekran koncowy z wynikami, 0 - gra trwa dalej

	uint8_t special_screen = screen_default; //Zmienna przechowujaca jaki szablon ekranu ma zostac wyswietlony

	//Zmienne informujace, czy wartosc jakiejsc zmiennej wyswietlanej na LCD ulegla zmianie. Jezeli nie, to jej nie wysylaj
	uint8_t change_special_screen = 0; //1 - wartośc uległa zmianie, prześlij ją na ekran LCD
	uint8_t change_number_of_points_USER = 0;
	uint8_t change_number_of_points_CPU = 0;
	uint8_t change_number_of_wins_USER = 0;
	uint8_t change_number_of_wins_CPU = 0;

	//Tablice informujace czy nastapila jakas zmiana w wartosci karty
	//1 - tak, wystapila, wiec nalezy ta zmiane przeslac do task3 w celu wyswietlenia na LCD
	//0 - nie, nie wystapila, wiec nie trzeba nic robic
	uint8_t change_card_U[10] = {0};
	uint8_t change_card_C[10] = {0};

	//Tablice przechowujace karty do wyswietlenia
	uint8_t card_to_show_U[10] = {0};
	uint8_t card_to_show_C[10] = {0};

	for(;;)
	{
		//Jezeli zostal wcisniety przycisk nowa gra---------------------------------------OBSLUGA PRZYCISKU!!!
		if( xSemaphoreTake( Semaphore_nowa_gra_pressed, ( TickType_t ) 10 ) == pdTRUE )
		{
			game_finish = 0;

			if (if_started_game == 1)
			{
				round_continues = 0; //Runda sie skonczyla
				if_board_1_prepare = 0; //Talia kart (tablica z wylosowanymi liczbami) nie jest gotowa
				nr_random_card_to_draw_board_1 = 0; //Umozliwi to losowanie kart
				//reset_random_card_board = 1;
				if_started_game = 0;
			}
			else if (if_board_1_prepare == 1)
			{
				if_started_game = 1;
				round_continues = 1; //Runda trwa
				nr_of_cards_taken = 0;

				//Ustawienia dla nowo rozpoczetej gry
				//number_of_wins_USER = 0; //Wyzerowanie liczby wygranych rund przez gracza
				change_number_of_wins_USER = 1; //Poinforuj o zmianie liczy wygranych rund przez gracza

				//number_of_wins_CPU = 0;  //Wyzerowanie liczby wygranych rund przez komputer
				change_number_of_wins_CPU = 1; //Poinforuj o zmianie liczy wygranych rund przez komputer

				allow_draw_a_card_USER = 1; //Odblokowanie mozliwosci dobierania kart dla uzytkownika
				allow_draw_a_card_CPU = 1;  //Odblokowanie mozliwosci dobierania kart dla komputera
				nr_cards_drawn_USER = 0; //Wyzerowanie liczby kart dobranych przez uzytkownika
				nr_cards_drawn_CPU = 0; //Wyzerowanie liczby kart dobranych przez komputer

				//Wyswietlenie domyslnego ekranu
				special_screen = screen_default;
				change_special_screen = 1;

				//Wylosowanie pierwszej oraz drugiej karty dla gracza-----------------------------------------
				number_of_points_USER = card_for_number( random_card_board_1[nr_of_cards_taken] );
				card_to_show_U[1] = random_card_board_1[nr_of_cards_taken];
				change_card_U[1] = 1; //Poinformuj o zmianie wartosci card_to_show_U[1] - to umozliwi jej wyswietlenie
				nr_of_cards_taken++;

				number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
				change_number_of_points_USER = 1;
				card_to_show_U[2] = random_card_board_1[nr_of_cards_taken];
				change_card_U[2] = 1;
				nr_of_cards_taken++;
				nr_cards_drawn_USER = 2;

				if(number_of_points_USER == 21) allow_draw_a_card_USER = 0;

				//Wylosowanie i wyswietlenie pierwszej oraz drugiej karty dla komputera---------------------------------------
				number_of_points_CPU = card_for_number( random_card_board_1[nr_of_cards_taken] );
				card_to_show_C[1] = random_card_board_1[nr_of_cards_taken];
				change_card_C[1] = 1;
				nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu

				number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
				change_number_of_points_CPU = 1;
				card_to_show_C[2] = random_card_board_1[nr_of_cards_taken];
				change_card_C[2] = 1;
				nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
				nr_cards_drawn_CPU = 2;

				if(number_of_points_CPU == 21) allow_draw_a_card_CPU = 0;
			}
			else
			{
				special_screen = screen_loading;
				change_special_screen = 1;
			}
		}//Koniec obslugi przycisku nowa gra

		//Jezeli zostal wcisniety przycisk kolejna karta----------------------------------OBSLUGA PRZYCISKU!!!
		if( xSemaphoreTake( Semaphore_kolejna_karta_pressed, ( TickType_t ) 10 ) == pdTRUE )
		{
			if (game_finish == 0)
			{
			//Jezeli runda sie juz skonczyla i chcemy przejsc do kolejnej
			if (round_continues == 0)
			{
				if (if_board_1_prepare == 1)
				{
					nr_of_cards_taken = 0;
					if_started_game = 1;
					round_continues = 1; //Rozpocznij runde
					special_screen = screen_clean_place_cards; //Wyczysc wylosowane karty
					change_special_screen = 1; //Poinformuj o koniecznosci wyswietlenia nowego szablonu

					allow_draw_a_card_USER = 1;
					allow_draw_a_card_CPU = 1;


					//Wylosowanie pierwszej oraz drugiej karty dla gracza-----------------------------------------
					number_of_points_USER = card_for_number( random_card_board_1[nr_of_cards_taken] );
					card_to_show_U[1] = random_card_board_1[nr_of_cards_taken];
					change_card_U[1] = 1; //Poinformuj o zmianie wartosci card_to_show_U[1] - to umozliwi jej wyswietlenie
					nr_of_cards_taken++;

					number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_USER = 1;
					card_to_show_U[2] = random_card_board_1[nr_of_cards_taken];
					change_card_U[2] = 1;
					nr_of_cards_taken++;
					nr_cards_drawn_USER = 2;

					if(number_of_points_USER == 21) allow_draw_a_card_USER = 0;

					//Wylosowanie i wyswietlenie pierwszej oraz drugiej karty dla komputera---------------------------------------
					number_of_points_CPU = card_for_number( random_card_board_1[nr_of_cards_taken] );
					card_to_show_C[1] = random_card_board_1[nr_of_cards_taken];
					change_card_C[1] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu

					number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_CPU = 1;
					card_to_show_C[2] = random_card_board_1[nr_of_cards_taken];
					change_card_C[2] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_CPU = 2;

					if(number_of_points_CPU == 21) allow_draw_a_card_CPU = 0;
				}
				else
				{
					special_screen = screen_loading;
					change_special_screen = 1;
				}
			}
			else //round continues == 1, ta sama runda dalej, trzeba dobrac karty
			{
				if(allow_draw_a_card_USER == 1)
				{
				switch (nr_cards_drawn_USER)
				{
				case 2:
				{
					number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_USER = 1;
					card_to_show_U[3] = random_card_board_1[nr_of_cards_taken];
					change_card_U[3] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_USER = 3;
					break;
				}
				case 3:
				{
					number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_USER = 1;
					card_to_show_U[4] = random_card_board_1[nr_of_cards_taken];
					change_card_U[4] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_USER = 4;
					break;
				}
				case 4:
				{
					number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_USER = 1;
					card_to_show_U[5] = random_card_board_1[nr_of_cards_taken];
					change_card_U[5] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_USER = 5;
					break;
				}
				case 5:
				{
					number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_USER = 1;
					card_to_show_U[6] = random_card_board_1[nr_of_cards_taken];
					change_card_U[6] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_USER = 6;
					break;
				}
				case 6:
				{
					number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_USER = 1;
					card_to_show_U[7] = random_card_board_1[nr_of_cards_taken];
					change_card_U[7] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_USER = 7;
					break;
				}
				case 7:
				{
					number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_USER = 1;
					card_to_show_U[8] = random_card_board_1[nr_of_cards_taken];
					change_card_U[8] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_USER = 8;
					break;
				}
				case 8:
				{
					number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_USER = 1;
					card_to_show_U[9] = random_card_board_1[nr_of_cards_taken];
					change_card_U[9] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_USER = 9;
					break;
				}

				}//End Switch User
				}//if alllow_user

				if (allow_draw_a_card_CPU == 1)
				{
				switch (nr_cards_drawn_CPU)
				{
				case 2:
				{
					number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_CPU = 1;
					card_to_show_C[3] = random_card_board_1[nr_of_cards_taken];
					change_card_C[3] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_CPU = 3;
					break;
				}
				case 3:
				{
					number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_CPU = 1;
					card_to_show_C[4] = random_card_board_1[nr_of_cards_taken];
					change_card_C[4] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_CPU = 4;
					break;
				}
				case 4:
				{
					number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_CPU = 1;
					card_to_show_C[5] = random_card_board_1[nr_of_cards_taken];
					change_card_C[5] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_CPU = 5;
					break;
				}
				case 5:
				{
					number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_CPU = 1;
					card_to_show_C[6] = random_card_board_1[nr_of_cards_taken];
					change_card_C[6] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_CPU = 6;
					break;
				}
				case 6:
				{
					number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_CPU = 1;
					card_to_show_C[7] = random_card_board_1[nr_of_cards_taken];
					change_card_C[7] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_CPU = 7;
					break;
				}
				case 7:
				{
					number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_CPU = 1;
					card_to_show_C[8] = random_card_board_1[nr_of_cards_taken];
					change_card_C[8] = 1;
					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_CPU = 8;
					break;
				}
				case 8:
				{
					number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
					change_number_of_points_CPU = 1;

					card_to_show_C[9] = random_card_board_1[nr_of_cards_taken];
					change_card_C[9] = 1;

					nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
					nr_cards_drawn_CPU = 9;
					break;
				}
				}//End Switch User
				}//if allow CPU
			}
			}//if game_finish

		}//Koniec obslugi przycisku kolejna karta

		//Jezeli zostal wcisniety przycisk reset------------------------------------------OBSLUGA PRZYCISKU!!!
		if( xSemaphoreTake( Semaphore_reset_pressed, ( TickType_t ) 10 ) == pdTRUE )
		{
			game_finish = 0;

			if (if_started_game == 1)
			{
				round_continues = 0; //Runda sie skonczyla
				if_board_1_prepare = 0; //Talia kart (tablica z wylosowanymi liczbami) nie jest gotowa
				nr_random_card_to_draw_board_1 = 0; //Umozliwi to losowanie kart
				//reset_random_card_board = 1;
				if_started_game = 0;
			}
			else if (if_board_1_prepare == 1)
			{
				if_started_game = 1;
				round_continues = 1; //Runda trwa
				nr_of_cards_taken = 0;

				//Ustawienia dla nowo rozpoczetej gry
				number_of_wins_USER = 0; //Wyzerowanie liczby wygranych rund przez gracza
				change_number_of_wins_USER = 1; //Poinforuj o zmianie liczy wygranych rund przez gracza

				number_of_wins_CPU = 0;  //Wyzerowanie liczby wygranych rund przez komputer
				change_number_of_wins_CPU = 1; //Poinforuj o zmianie liczy wygranych rund przez komputer

				allow_draw_a_card_USER = 1; //Odblokowanie mozliwosci dobierania kart dla uzytkownika
				allow_draw_a_card_CPU = 1;  //Odblokowanie mozliwosci dobierania kart dla komputera
				nr_cards_drawn_USER = 0; //Wyzerowanie liczby kart dobranych przez uzytkownika
				nr_cards_drawn_CPU = 0; //Wyzerowanie liczby kart dobranych przez komputer

				//Wyswietlenie domyslnego ekranu
				special_screen = screen_default;
				change_special_screen = 1;

				//Wylosowanie pierwszej oraz drugiej karty dla gracza-----------------------------------------
				number_of_points_USER = card_for_number( random_card_board_1[nr_of_cards_taken] );
				card_to_show_U[1] = random_card_board_1[nr_of_cards_taken];
				change_card_U[1] = 1; //Poinformuj o zmianie wartosci card_to_show_U[1] - to umozliwi jej wyswietlenie
				nr_of_cards_taken++;

				number_of_points_USER += card_for_number( random_card_board_1[nr_of_cards_taken] );
				change_number_of_points_USER = 1;
				card_to_show_U[2] = random_card_board_1[nr_of_cards_taken];
				change_card_U[2] = 1;
				nr_of_cards_taken++;
				nr_cards_drawn_USER = 2;

				if(number_of_points_USER == 21) allow_draw_a_card_USER = 0;

				//Wylosowanie i wyswietlenie pierwszej oraz drugiej karty dla komputera---------------------------------------
				number_of_points_CPU = card_for_number( random_card_board_1[nr_of_cards_taken] );
				card_to_show_C[1] = random_card_board_1[nr_of_cards_taken];
				change_card_C[1] = 1;
				nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu

				number_of_points_CPU += card_for_number( random_card_board_1[nr_of_cards_taken] );
				change_number_of_points_CPU = 1;
				card_to_show_C[2] = random_card_board_1[nr_of_cards_taken];
				change_card_C[2] = 1;
				nr_of_cards_taken++; //Zwieksz o jeden liczbe wzietych kart ze stosu
				nr_cards_drawn_CPU = 2;

				if(number_of_points_CPU == 21) allow_draw_a_card_CPU = 0;
				}
				else
				{
					special_screen = screen_loading;
					change_special_screen = 1;
				}
		}


		//Jezeli zostal wcisniety przycisk wystarczy kart
		if( xSemaphoreTake( Semaphore_wystarczy_kart_pressed, ( TickType_t ) 10 ) == pdTRUE )
		{
			if (game_finish == 0)
			{
				allow_draw_a_card_USER = 0; //Zabroń dobierania kart graczowi
			}
		}

		//Kiedy zakonczyc runde?
		//Jezeli gracz ma wiecej badz równo 21 punktow
		//gdy komputer ma większą bądź równą wartość zadanych punktów
		if (number_of_points_USER >= 21) allow_draw_a_card_USER = 0;
		if (number_of_points_CPU > value_fight_CPU) allow_draw_a_card_CPU = 0;


		//Jezeli zaistnieje warunek do zakonczenia rundy to:
		if ( (allow_draw_a_card_USER == 0) && (allow_draw_a_card_CPU == 0) &&(if_started_game == 1) )
		{
			{
				round_continues = 0; //Runda sie skończyła
				if_board_1_prepare = 0; //Talia kart (tablica z wylosowanymi liczbami) nie jest gotowa
				nr_random_card_to_draw_board_1 = 0; //Umożliwi to losowanie kart
				if_started_game = 0;
			}

			//Kto wygral RUNDE? Przyznaj punkt zwyciezcy
			//Jezeli komputer oraz gracz maja powyzej 21 punktow, ale gracz ma mniej punktow to wygrywa gracz itp.
				 if ((number_of_points_USER > 21) && (number_of_points_CPU > 21) && (number_of_points_USER < number_of_points_CPU))
				 	{ number_of_wins_USER++; change_number_of_wins_USER = 1;
				 	  special_screen = screen_winner_this_round_U; change_special_screen = 1; }
			else if ((number_of_points_USER > 21) && (number_of_points_CPU > 21) && (number_of_points_USER > number_of_points_CPU))
					{ number_of_wins_CPU++; change_number_of_wins_CPU = 1;
					  special_screen = screen_winner_this_round_C; change_special_screen = 1; }
			else if ((number_of_points_USER < 21) && (number_of_points_CPU < 21) && (number_of_points_USER > number_of_points_CPU))
					{ number_of_wins_USER++; change_number_of_wins_USER = 1;
					  special_screen = screen_winner_this_round_U; change_special_screen = 1; }
			else if ((number_of_points_USER < 21) && (number_of_points_CPU < 21) && (number_of_points_USER < number_of_points_CPU))
					{ number_of_wins_CPU++; change_number_of_wins_CPU = 1;
					  special_screen = screen_winner_this_round_C; change_special_screen = 1; }
			else if ((number_of_points_USER == 21) && (number_of_points_CPU != 21))
					{ number_of_wins_USER++; change_number_of_wins_USER = 1;
					  special_screen = screen_winner_this_round_U; change_special_screen = 1; }
			else if ((number_of_points_USER != 21) && (number_of_points_CPU == 21))
					{ number_of_wins_CPU++; change_number_of_wins_CPU = 1;
					  special_screen = screen_winner_this_round_C; change_special_screen = 1; }
			else if ( (number_of_points_USER == number_of_points_CPU) && (number_of_points_USER != 0))
					{ special_screen = screen_winner_this_round_D; change_special_screen = 1; }
			else if ((number_of_points_USER < 21) && (number_of_points_CPU > 21))
					{ number_of_wins_USER++; change_number_of_wins_USER = 1;
					  special_screen = screen_winner_this_round_U; change_special_screen = 1; }
			else if ((number_of_points_USER > 21) && (number_of_points_CPU < 21))
					{ number_of_wins_CPU++; change_number_of_wins_CPU = 1;
					  special_screen = screen_winner_this_round_C; change_special_screen = 1; }

			//Kto wygral cala gre? Ten kto zwyciężył w większej ilości rund
			if ((number_of_wins_USER == amount_rounds_to_win_game) || (number_of_wins_CPU == amount_rounds_to_win_game))
			{
				uint8_t winner_this_epic_game;
				if (number_of_wins_USER > number_of_wins_CPU)
				{
					winner_this_epic_game = screen_winner_this_game_U;
					xQueueSend(winner_the_gameLCDqueue, &winner_this_epic_game, time_to_wait);
				}
				else
				{
					winner_this_epic_game = screen_winner_this_game_C;
					xQueueSend(winner_the_gameLCDqueue, &winner_this_epic_game, time_to_wait);
				}
				xQueueSend(wins_U_end_LCDqueue, &number_of_wins_USER, time_to_wait); //Wyświetl liczbę wygranych rund przez gracza na ekranie podsumowywującym grę
				xQueueSend(wins_C_end_LCDqueue, &number_of_wins_CPU, time_to_wait);  //Wyświetl liczbę wygranych rund przez komputer na ekranie podsumowywującym grę
				game_finish = 1; //Gra skończona
				number_of_wins_USER = 0; //Wyzeruj liczbę wygranych rund przez gracza
				number_of_wins_CPU = 0;  //Wyzeruj liczbę wygranych rund przez komputer
			}
		}


		//--------------WYSYLANIE DANYCH DO WYSWIETLENIA NA LCD----------------------------------
		//Sprawdź za pomocą zmiennych change... wartości których zmiennych uległy zmianie
		//Jeżeli wartość jest taka sama jak poprzednio to nie wysyłaj tego do LCD (Task3)
		//Natomiast jeżeli wartość się zmieniła to wyślij to na LCD

		//Wyświetlenie domyślnego ekranu tzw. szablonu
		if (change_special_screen == 1) { LCD_show_template; change_special_screen = 0; }

		//Wyświetlenie wylosowanych kart
		//przez gracza
		if (change_card_U[1] == 1) { LCD_show_card1_U; change_card_U[1] = 0; }
		if (change_card_U[2] == 1) { LCD_show_card2_U; change_card_U[2] = 0; }
		if (change_card_U[3] == 1) { LCD_show_card3_U; change_card_U[3] = 0; }
		if (change_card_U[4] == 1) { LCD_show_card4_U; change_card_U[4] = 0; }
		if (change_card_U[5] == 1) { LCD_show_card5_U; change_card_U[5] = 0; }
		if (change_card_U[6] == 1) { LCD_show_card6_U; change_card_U[6] = 0; }
		if (change_card_U[7] == 1) { LCD_show_card7_U; change_card_U[7] = 0; }
		if (change_card_U[8] == 1) { LCD_show_card8_U; change_card_U[8] = 0; }
		if (change_card_U[9] == 1) { LCD_show_card9_U; change_card_U[9] = 0; }
		//przez komputer
		if (change_card_C[1] == 1) { LCD_show_card1_C; change_card_C[1] = 0; }
		if (change_card_C[2] == 1) { LCD_show_card2_C; change_card_C[2] = 0; }
		if (change_card_C[3] == 1) { LCD_show_card3_C; change_card_C[3] = 0; }
		if (change_card_C[4] == 1) { LCD_show_card4_C; change_card_C[4] = 0; }
		if (change_card_C[5] == 1) { LCD_show_card5_C; change_card_C[5] = 0; }
		if (change_card_C[6] == 1) { LCD_show_card6_C; change_card_C[6] = 0; }
		if (change_card_C[7] == 1) { LCD_show_card7_C; change_card_C[7] = 0; }
		if (change_card_C[8] == 1) { LCD_show_card8_C; change_card_C[8] = 0; }
		if (change_card_C[9] == 1) { LCD_show_card9_C; change_card_C[9] = 0; }

		//Wyświetlenie liczby zdobytych punktów przez gracza
		if (change_number_of_points_USER == 1) { LCD_show_points_U; change_number_of_points_USER = 0; }

		//Wyświetlenie liczby zdobytych punktów przez komputer
		if (change_number_of_points_CPU == 1) {	LCD_show_points_C; change_number_of_points_CPU = 0;	}

		//Wyświetlenie liczby wygranych rund gracza
		if (change_number_of_wins_USER == 1) { LCD_show_round_wins_U; change_number_of_wins_USER = 0; }

		//Wyświetlenie liczby wygranych rund komputera
		if (change_number_of_wins_CPU == 1) { LCD_show_round_wins_C; change_number_of_wins_CPU = 0; }

		osDelay(100);
	}
}


/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  //Obsluga przerwania od przepelnienia TIM4

  //Obsluga przyciskow z uzyciem debouncingu oraz losowanie kart (liczb) i wpisywanie ich do tablicy
  if (htim->Instance == TIM4)
  {
	  if (nr_random_card_to_draw_board_1 < 52) //Jeżeli liczba wylosowanych kart jest mniejsza niż 52 (52 - liczba kart w talii)
	  {
		  if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) //Jeżeli przekonwetowano odczytaną wartość analogową na cyfrową
		  {
			  ADC_measure = ((HAL_ADC_GetValue(&hadc1)) % 12) + 2; //"Wylosuj" liczbę z zakresu od 2 do 14
			  HAL_ADC_Start(&hadc1); //Rozpocznik kolejny pomiar przetwornikiem ADC

			  assignment_card_to_board = 0; //1 - przypisano wylosowaną "kartę", 0 - wylosowana karta nie została przypisana
			  while (assignment_card_to_board == 0) //Dopóki wylosowana karta nie została przypisana to
			  {
				  switch (ADC_measure)
				  {
				  case 2: { if (number_of_cards_2 > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 2; number_of_cards_2--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 3: { if (number_of_cards_3 > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 3; number_of_cards_3--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 4: { if (number_of_cards_4 > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 4; number_of_cards_4--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 5: { if (number_of_cards_5 > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 5; number_of_cards_5--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 6: { if (number_of_cards_6 > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 6; number_of_cards_6--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 7: { if (number_of_cards_7 > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 7; number_of_cards_7--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 8: { if (number_of_cards_8 > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 8; number_of_cards_8--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 9: { if (number_of_cards_9 > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 9; number_of_cards_9--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 10: { if (number_of_cards_T > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 10; number_of_cards_T--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 11: { if (number_of_cards_J > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 11; number_of_cards_J--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 12: { if (number_of_cards_Q > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 12; number_of_cards_Q--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 13: { if (number_of_cards_K > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 13; number_of_cards_K--; assignment_card_to_board = 1;} else ADC_measure++; break;}
				  case 14: { if (number_of_cards_A > 0) {random_card_board_1[nr_random_card_to_draw_board_1] = 14; number_of_cards_A--; assignment_card_to_board = 1;} else ADC_measure = 2; break;}
				  }//Koniec switcha
			  }

			  //Zapis do tablicy wylosowanej karty na co czwartej pozycji 0,4,...,48,1,5,...,49,2,6,...,50,3,7,...,51
			  nr_random_card_to_draw_board_1 = nr_random_card_to_draw_board_1 + 4;
			  if (nr_random_card_to_draw_board_1 == 52) nr_random_card_to_draw_board_1 = 1;
			  if (nr_random_card_to_draw_board_1 == 53) nr_random_card_to_draw_board_1 = 2;
			  if (nr_random_card_to_draw_board_1 == 54) nr_random_card_to_draw_board_1 = 3;
		  }

		  if(nr_random_card_to_draw_board_1 == 55) //Jeżeli zakończono losowanie kart spowodowane brakiem kart do losowania to
		  {
			  if_board_1_prepare = 1; //Informuje, ze tablica jest z wylosowanymi liczba jest gotowa do użycia
			  //Uzupełnij talię kartami, aby można było z niej losować
			  number_of_cards_2 = 4;
			  number_of_cards_3 = 4;
			  number_of_cards_4 = 4;
			  number_of_cards_5 = 4;
			  number_of_cards_6 = 4;
			  number_of_cards_7 = 4;
			  number_of_cards_8 = 4;
			  number_of_cards_9 = 4;
			  number_of_cards_T = 4;
			  number_of_cards_J = 4;
			  number_of_cards_Q = 4;
			  number_of_cards_K = 4;
			  number_of_cards_A = 4;
		  }
	  }

	  //Obsługa i eliminacja debouncingu dla uzywanych czterech przyciskow
	  if (HAL_GPIO_ReadPin(Przycisk_nowa_gra_GPIO_Port, Przycisk_nowa_gra_Pin) == GPIO_PIN_RESET)
	  {
		  DebCount_nowa_gra++;
		  DebCount_reset = 0;
		  DebCount_wystarczy_kart = 0;
		  DebCount_kolejna_karta = 0;
	  }
	  else if (HAL_GPIO_ReadPin(Przycisk_reset_GPIO_Port, Przycisk_reset_Pin) == GPIO_PIN_RESET)
	  {
		  DebCount_nowa_gra = 0;
		  DebCount_reset++;
		  DebCount_wystarczy_kart = 0;
		  DebCount_kolejna_karta = 0;
	  }
	  else if (HAL_GPIO_ReadPin(Przycisk_wystarczy_kart_GPIO_Port, Przycisk_wystarczy_kart_Pin) == GPIO_PIN_RESET)
	  {
		  DebCount_nowa_gra = 0;
		  DebCount_reset = 0;
		  DebCount_wystarczy_kart++;
		  DebCount_kolejna_karta = 0;
	  }
	  else if (HAL_GPIO_ReadPin(Przycisk_kolejna_karta_GPIO_Port, Przycisk_kolejna_karta_Pin) == GPIO_PIN_RESET)
	  {
		  DebCount_nowa_gra = 0;
		  DebCount_reset = 0;
		  DebCount_wystarczy_kart = 0;
		  DebCount_kolejna_karta++;
	  }
	  else
	  {
		  DebCount_nowa_gra = 0;
		  DebCount_reset = 0;
		  DebCount_wystarczy_kart = 0;
		  DebCount_kolejna_karta = 0;

		  Switched_nowa_gra = 0;
		  Switched_reset = 0;
		  Switched_wystarczy_kart = 0;
		  Switched_kolejna_karta = 0;
	  }

	  if ( (DebCount_nowa_gra >= time_to_press) && (Switched_nowa_gra == 0) )
	  {
		  Switched_nowa_gra = 1;
		  Pressed_nowa_gra = 1;
	  }
	  else if ( (DebCount_reset >= time_to_press) && (Switched_reset == 0) )
	  {
		  Switched_reset = 1;
		  Pressed_reset = 1;
	  }
	  else if ( (DebCount_wystarczy_kart >= time_to_press) && (Switched_wystarczy_kart == 0) )
	  {
		  Switched_wystarczy_kart = 1;
		  Pressed_wystarczy_kart = 1;
	  }
	  else if ( (DebCount_kolejna_karta >= time_to_press) && (Switched_kolejna_karta == 0) )
	  {
		  Switched_kolejna_karta = 1;
		  Pressed_kolejna_karta = 1;
	  }
  }


  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

