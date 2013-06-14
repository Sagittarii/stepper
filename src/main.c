#include "stm32f4xx_conf.h"


uint32_t freq_request = 1;
uint32_t freq_current = 1;

void TIM2_IRQHandler(void)
{
    static int direction_up = 1;

    if (TIM2->SR & TIM_SR_UIF)
    {
        GPIOD->ODR ^= (1 << 9);

        if (direction_up == 1)
        {
            freq_current++;
        }
        else
        {
            freq_current--;
        }

        if (freq_current >= freq_request)
        {
            direction_up = 0;
        }
        if (freq_current == 0)
        {
            direction_up = 1;
        }

        //TIM2->ARR = freq_current;
        TIM2->ARR = (uint32_t) ( (float)0x1000 / (float) freq_current) + 3;

    }
    TIM2->SR = 0x0; // reset the status register
}

void TIM4_IRQHandler(void)
{

    static int dir_up = 1;
  // flash on update event
  if (TIM4->SR & TIM_SR_UIF)
  {
      //GPIOD->ODR ^= (1 << 13);
      if (dir_up == 1)
      {
          TIM4->CCR1 += 0x01;
      }
      else
      {
          TIM4->CCR1 -= 0x01;
      }

      if (TIM4->CCR1 >= 0x0FF)
      {
          dir_up = 0;
      }
      if (TIM4->CCR1 == 0x000)
      {
          dir_up = 1;
      }

      TIM4->CCR2 = TIM4->CCR1;
      TIM4->CCR3 = 0x100 - TIM4->CCR1;
      //TIM4->CCR4 = 0x100 - TIM4->CCR1;
      TIM4->CCR4 = TIM4->CCR1;
  }
   
  TIM4->SR = 0x0; // reset the status register
}
 

void EXTI0_IRQHandler(void)
{
    TIM4->CCR1 = 0x001;
    TIM4->CCR2 = TIM4->CCR3 = TIM4->CCR4 = TIM4->CCR1;

    TIM2->ARR -= (TIM2->ARR / 20);
    freq_request ++;// (freq_request /20);

    //GPIOD->ODR ^= (1 << 13);

    //EXTI->PR |= EXTI_PR_PR0; // reset the interrupt
    EXTI_ClearITPendingBit(EXTI_Line0);
}


void EXTI9_5_IRQHandler(void)
{
    TIM4->CCR1 = 0x001;
    TIM4->CCR2 = TIM4->CCR3 = TIM4->CCR4 = TIM4->CCR1;

    TIM2->ARR -= (TIM2->ARR / 20);
    freq_request ++;// (freq_request /20);

    //GPIOD->ODR ^= (1 << 13);

    //EXTI->PR |= EXTI_PR_PR0; // reset the interrupt
    EXTI_ClearITPendingBit(EXTI_Line7);
    EXTI_ClearITPendingBit(EXTI_Line9);
}

int main(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN; // enable the clock to GPIOD
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN; // enable TIM2 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN; // enable TIM4 clock
     
    GPIOD->MODER = (1 << (2*9)) + (2 << (2*12) ) + (2 << (2*13) ) + (2 << (2*14) ) + (2 << (2*15) ); // set pin 13 to be alternate function mode
    GPIOD->AFR[1] = (2 << 16) + (2 << 20) + (2 << 24) + (2 << 28); // pin 13 in alternate function AF2

    GPIOD->ODR ^= (1 << 13);

    // Enable FPU
    SCB->CPACR |= ((3UL << 10*2)|(3UL << 11*2));  /* set CP10 and CP11 Full Access */

    // Timer 2 : stepper
    NVIC->ISER[0] |= 1 << (TIM2_IRQn); // enable the TIM2 IRQ

    TIM2->PSC = 0x0008; // no prescaler, timer counts up in sync with the peripheral clock
    TIM2->DIER |= TIM_DIER_UIE; // enable update interrupt
    TIM2->ARR = 0x1000; // count to 1 (autoreload value 1)
    freq_current = 1;
    freq_request = 1;

    TIM2->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN; // autoreload on, counter enabled
    TIM2->EGR = 1; // trigger update event to reload timer registers


    // Timer 4
    NVIC->ISER[0] |= 1 << (TIM4_IRQn); // enable the TIM4 IRQ
     
    TIM4->PSC = 0x0080; // no prescaler, timer counts up in sync with the peripheral clock
    TIM4->DIER |= TIM_DIER_UIE; // enable update interrupt
    TIM4->ARR = 0x800; // count to 1 (autoreload value 1)
    TIM4->CCR1 = 0x000;// PWM value
    TIM4->CCR2 = 0x200;// PWM value
    TIM4->CCR3 = 0x400;// PWM value
    TIM4->CCR4 = 0x600;// PWM value

    TIM4->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN; // autoreload on, counter enabled

    TIM4->EGR = 1; // trigger update event to reload timer registers

    TIM4->CCER |= TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E; // enable OC1 as Output
    TIM4->CCMR1 |=   TIM_CCMR1_OC1M_1 | TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1PE
                   | TIM_CCMR1_OC2M_1 | TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2PE;
    TIM4->CCMR2 |=   TIM_CCMR2_OC3M_1 | TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3PE
                   | TIM_CCMR2_OC4M_1 | TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4PE;

    // External interrupt on PA0

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
//    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

//    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOA, EXTI_PinSource0);

//    GPIO_InitTypeDef   GPIO_InitStructure;
//    EXTI_InitTypeDef   EXTI_InitStructure;
//    NVIC_InitTypeDef   NVIC_InitStructure;

//    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
//    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
//    GPIO_Init(GPIOA, &GPIO_InitStructure);

//    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
//    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
//    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
//    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
//    EXTI_Init(&EXTI_InitStructure);

//    NVIC_InitStructure.NVIC_IRQChannel = EXTI0_IRQn;
//    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x0F;
//    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x0F;
//    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//    NVIC_Init(&NVIC_InitStructure);


    // External interrupt on PE9
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);

    GPIO_InitTypeDef   GPIO_InitStructureE9;
    EXTI_InitTypeDef   EXTI_InitStructureE9;
    NVIC_InitTypeDef   NVIC_InitStructureE9;

    GPIO_InitStructureE9.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructureE9.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructureE9.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOE, &GPIO_InitStructureE9);

    EXTI_InitStructureE9.EXTI_Line = EXTI_Line9;
    EXTI_InitStructureE9.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructureE9.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructureE9.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructureE9);

    NVIC_InitStructureE9.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructureE9.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructureE9.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructureE9.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructureE9);

    // External interrupt on PE9
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOE, EXTI_PinSource9);

    GPIO_InitTypeDef   GPIO_InitStructureE7;
    EXTI_InitTypeDef   EXTI_InitStructureE7;
    NVIC_InitTypeDef   NVIC_InitStructureE7;

    GPIO_InitStructureE7.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructureE7.GPIO_PuPd = GPIO_PuPd_DOWN;
    GPIO_InitStructureE7.GPIO_Pin = GPIO_Pin_7;
    GPIO_Init(GPIOE, &GPIO_InitStructureE7);

    EXTI_InitStructureE7.EXTI_Line = EXTI_Line7;
    EXTI_InitStructureE7.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructureE7.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructureE7.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructureE7);

    NVIC_InitStructureE7.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStructureE7.NVIC_IRQChannelPreemptionPriority = 0x0F;
    NVIC_InitStructureE7.NVIC_IRQChannelSubPriority = 0x0F;
    NVIC_InitStructureE7.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructureE7);

    /*
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN; // enable the clock to GPIOA
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN; // enable clock for SYSCFG

    GPIOA->MODER = 0;

    EXTI->IMR = EXTI_IMR_MR0; // don't mask interrupt request from line 0
    EXTI->EMR = EXTI_EMR_MR0;//0; // Mask all events
    EXTI->RTSR = EXTI_RTSR_TR0; // Rising trigger on line 0.
    EXTI->RTSR = 0; // No falling trigger
    EXTI->SWIER |= EXTI_SWIER_SWIER0; // Don't use this, it is for generating interrupt manually by software
    //EXTI->PR |= EXTI_PR_PR0; // To clear the pending request
    SYSCFG->EXTICR[0] = SYSCFG_EXTICR1_EXTI0_PA; // Set External interrupt line 0 on PA0 pin
    NVIC->ISER[0] |= 1 << (EXTI0_IRQn); // enable the EXTI Line 0 IRQ
*/

    while (1)
    {
        //if( (GPIOA->IDR & 0x01) == 0x01)
        //if( (GPIOE->IDR & (1<<9)) != 0x00)
//        {
//            TIM4->CCR1 = 0x001;
//            TIM4->CCR2 = TIM4->CCR3 = TIM4->CCR4 = TIM4->CCR1;
//        }
    }
}
