/**
 * SPDX-License-Identifier: GPL-2.0
 * Copyright (c) Bao Project and Contributors. All rights reserved.
 */

#include "rzt2m_uart.h"
#include <stddef.h>
#include "ioport.h"
#include <cpu.h>

/***********************************************************************************************************************
 * Macro definitions
 **********************************************************************************************************************/

#define BSP_PRV_PRCR_KEY    (0xA500U)

#define SCI_UART_CCR0_DEFAULT_VALUE           (0x00000000)
#define SCI_UART_CCR1_DEFAULT_VALUE           (0x00000010)
#define SCI_UART_CCR2_DEFAULT_VALUE           (0xFF00FF04)
#define SCI_UART_CCR3_DEFAULT_VALUE           (0x00001203)
#define SCI_UART_CCR4_DEFAULT_VALUE           (0x00000000)

#define SCI_UART_BRR_DEFAULT_VALUE            (0xFFU)
#define SCI_UART_FCR_DEFAULT_VALUE            (0x1F1F0000)

/** SCI CCR3 register bit masks */
#define SCI_UART_CCR3_BPEN_OFFSET             (7U)
#define SCI_UART_CCR3_CHR_OFFSET              (8U)
#define SCI_UART_CCR3_STP_OFFSET              (14U)
#define SCI_UART_CCR3_RxDSEL_OFFSET           (15U)
#define SCI_UART_CCR3_CKE_OFFSET              (24U)
#define SCI_UART_CCR3_CKE_VALUE_MASK          (0x03U)

/** SCI CCR2 register bit offsets */
#define SCI_UART_CCR2_BRME_OFFSET             (16U)
#define SCI_UART_CCR2_ABCSE_OFFSET            (6U)
#define SCI_UART_CCR2_ABCS_OFFSET             (5U)
#define SCI_UART_CCR2_BDGM_OFFSET             (4U)
#define SCI_UART_CCR2_CKS_OFFSET              (20U)
#define SCI_UART_CCR2_CKS_VALUE_MASK          (0x03U) ///< CKS: 2 bits
#define SCI_UART_CCR2_BRR_OFFSET              (8U)
#define SCI_UART_CCR2_BRR_VALUE_MASK          (0xFFU) ///< BRR: 8bits
#define SCI_UART_CCR2_MDDR_OFFSET             (24U)
#define SCI_UART_CCR2_MDDR_VALUE_MASK         (0xFFU) ///< MDDR: 8bits



/** SCI CCR0 register bit masks */
#define SCI_UART_CCR0_IDSEL_MASK              (0x00000400)
#define SCI_UART_CCR0_TEIE_MASK               (0x00200000)
#define SCI_UART_CCR0_RE_MASK                 (0x00000001)
#define SCI_UART_CCR0_TE_MASK                 (0x00000010)
#define SCI_UART_CCR0_RIE_MASK                (0x00010000)
#define SCI_UART_CCR0_TIE_MASK                (0x00100000)

#define Channel1to3_selector 0x3
#define Channel5_selector 5
#define SCI_Secure_mask 24
#define SCI_NonSecure_mask 10


static const uint16_t g_prcr_masks[] =
{
    0x0001U,                           /* PRC0. */
    0x0002U,                           /* PRC1. */
    0x0004U,                           /* PRC2. */
    0x0008U,                           /* PRC3. */
};



/** SCI CCR1 register bit offsets */
#define SCI_UART_CCR1_CTSE_OFFSET             (0U)
#define SCI_UART_CCR1_SPB2DT_BIT              (4U)
#define SCI_UART_CCR1_OUTPUT_ENABLE_MASK      (0x00000020)
#define SCI_UART_CCR1_PE_OFFSET               (8U)
#define SCI_UART_CCR1_PM_PE_VALUE_MASK        (0x03U)
#define SCI_UART_CCR1_NFCS_OFFSET             (24U)
#define SCI_UART_CCR1_NFCS_VALUE_MASK         (0x07U)
#define SCI_UART_CCR1_NFEN_OFFSET             (28U)


#define SCI_UART_CCR2_BAUD_SETTING_MASK       ((1U << SCI_UART_CCR2_BRME_OFFSET) |                          \
                                               (1U << SCI_UART_CCR2_ABCSE_OFFSET) |                         \
                                               (1U << SCI_UART_CCR2_ABCS_OFFSET) |                          \
                                               (1U << SCI_UART_CCR2_BDGM_OFFSET) |                          \
                                               (SCI_UART_CCR2_CKS_VALUE_MASK << SCI_UART_CCR2_CKS_OFFSET) | \
                                               (SCI_UART_CCR2_BRR_VALUE_MASK << SCI_UART_CCR2_BRR_OFFSET) | \
                                               (SCI_UART_CCR2_MDDR_VALUE_MASK << SCI_UART_CCR2_MDDR_OFFSET))



ioport_instance_ctrl_t g_ioport_ctrl;

ioport_extend_cfg_t g_ioport_cfg_extend;


const ioport_cfg_t g_bsp_pin_cfg =
{ .number_of_pins = sizeof(g_bsp_pin_cfg_data) / sizeof(ioport_pin_cfg_t),
  .p_pin_cfg_data = &g_bsp_pin_cfg_data[0],
  .p_extend = &g_ioport_cfg_extend, };

static volatile uint16_t g_protect_counters[] =
{
    0U, 0U, 0U, 0U
};

void register_protect_disable (bsp_reg_protect_t regs_to_unprotect)
{
    /* If this is first entry then disable protection. */
    if (0U == g_protect_counters[regs_to_unprotect])
    {
        /** Disable protection using PRCR register. */

        /** When writing to the PRCR register the upper 8-bits must be the correct key. Set lower bits to 0 to
         * disable writes. */
        R_RWP_NS->PRCRN = ((R_RWP_NS->PRCRN | BSP_PRV_PRCR_KEY) | g_prcr_masks[regs_to_unprotect]);
        R_RWP_S->PRCRS  = ((R_RWP_S->PRCRS | BSP_PRV_PRCR_KEY) | g_prcr_masks[regs_to_unprotect]);
    }

    /** Increment the protect counter */
    g_protect_counters[regs_to_unprotect]++;
}


static void r_sci_uart_baud_set (volatile R_SCI0_Type * p_sci_reg, baud_setting_t const * const p_baud_setting)
{
    p_sci_reg->CCR2 = (uint32_t) ((p_sci_reg->CCR2 & ~(SCI_UART_CCR2_BAUD_SETTING_MASK)) |
                                  (p_baud_setting->ccr2_baudrate_bits & SCI_UART_CCR2_BAUD_SETTING_MASK));
}




static void uart_config_set (volatile R_SCI0_Type * const p_uart, uart_cfg_t const * const p_cfg)
{
    uint32_t ccr3 = SCI_UART_CCR3_DEFAULT_VALUE;
    if (BSP_FEATURE_SCI_UART_FIFO_CHANNELS & (1U << p_cfg->channel))
    {
        p_uart->FCR = SCI_UART_FCR_DEFAULT_VALUE; 
    }
    
    /* Configure data size. */
    if (UART_DATA_BITS_7 == p_cfg->data_bits)
    {
        ccr3 |= (1U << SCI_UART_CCR3_CHR_OFFSET); 
    }
    else if (UART_DATA_BITS_9 == p_cfg->data_bits)
    {
        ccr3 &= ~(1U << (SCI_UART_CCR3_CHR_OFFSET + 1));
    }
    else
    {
        /* Do nothing.  Default is 8-bit mode. */
    }

        /* Configure stop bits. */
    ccr3 |= (uint32_t) p_cfg->stop_bits << SCI_UART_CCR3_STP_OFFSET;

    /* Configure CKE bits. */
    sci_uart_extended_cfg_t * p_extend = (sci_uart_extended_cfg_t *) p_cfg->p_extend;
    ccr3 |= (p_extend->clock & SCI_UART_CCR3_CKE_VALUE_MASK) << SCI_UART_CCR3_CKE_OFFSET;

    /* Starts reception on falling edge of RXD if enabled in extension (otherwise reception starts at low level
     * of RXD). */
    ccr3 |= (p_extend->rx_edge_start & 1U) << SCI_UART_CCR3_RxDSEL_OFFSET;

    /* Configure SPEN bit. */
    ccr3 |= (p_extend->sync_bypass & 1U) << SCI_UART_CCR3_BPEN_OFFSET;

    /* Write to the CCR3 register. */
    p_uart->CCR3 = ccr3;

    /* CCR1 register setting. */
    uint32_t ccr1 = SCI_UART_CCR1_DEFAULT_VALUE;

    /* Set the default level of the TX pin to 1. */
    ccr1 |= (uint32_t) (1U << SCI_UART_CCR1_SPB2DT_BIT | SCI_UART_CCR1_OUTPUT_ENABLE_MASK);

    /* Configure parity bits. */
    ccr1 |= (uint32_t) (p_cfg->parity & SCI_UART_CCR1_PM_PE_VALUE_MASK) << SCI_UART_CCR1_PE_OFFSET;

    /* Configure CTS flow control if CTS/RTS flow control is enabled. */
    ccr1 |= (uint32_t) (p_extend->ctsrts_en << SCI_UART_CCR1_CTSE_OFFSET);

    if (SCI_UART_NOISE_CANCELLATION_DISABLE != p_extend->noise_cancel) 
    {
        /* Select noise filter clock */
        ccr1 |= (uint32_t) (((p_extend->noise_cancel & 0x07U) - 1) & SCI_UART_CCR1_NFCS_VALUE_MASK) <<
                SCI_UART_CCR1_NFCS_OFFSET;

        /* Enables the noise cancellation */
        ccr1 |= (uint32_t) 1 << SCI_UART_CCR1_NFEN_OFFSET;
    }

    p_uart->CCR1 = ccr1;

    if ((SCI_UART_CLOCK_EXT8X == p_extend->clock) || (SCI_UART_CLOCK_EXT16X == p_extend->clock))
    {
        /* Use external clock for baud rate */
        p_uart->CCR2_b.BRR = SCI_UART_BRR_DEFAULT_VALUE;

        if (SCI_UART_CLOCK_EXT8X == p_extend->clock)
        {
            /* Set baud rate as (external clock / 8) */
            p_uart->CCR2 |= 1U << SCI_UART_CCR2_ABCS_OFFSET;
        }
    }
    else
    {
        /* Set the baud rate settings for the internal baud rate generator. */
        r_sci_uart_baud_set(p_uart, p_extend->p_baud_setting); 
    }



}



/*******************************************************************************************************************//**
 *        Enable register protection. Registers that are protected cannot be written to. Register protection is
 *          enabled by using the Protect Register (PRCR) and the MPC's Write-Protect Register (PWPR).
 *
 * @param[in] regs_to_protect Registers which have write protection enabled.
 **********************************************************************************************************************/
void register_protect_enable (bsp_reg_protect_t regs_to_protect)
{
    /* Is it safe to disable write access? */
    if (0U != g_protect_counters[regs_to_protect])
    {
        /* Decrement the protect counter */
        g_protect_counters[regs_to_protect]--;
    }

    /* Is it safe to disable write access? */
    if (0U == g_protect_counters[regs_to_protect])
    {
        /** Enable protection using PRCR register. */

        /** When writing to the PRCR register the upper 8-bits must be the correct key. Set lower bits to 0 to
         * disable writes. */
        R_RWP_NS->PRCRN = ((R_RWP_NS->PRCRN | BSP_PRV_PRCR_KEY) & (uint16_t) (~g_prcr_masks[regs_to_protect]));
        R_RWP_S->PRCRS  = ((R_RWP_S->PRCRS | BSP_PRV_PRCR_KEY) & (uint16_t) (~g_prcr_masks[regs_to_protect]));
    }
}


volatile bao_uart_t *system_mem1;
volatile bao_uart_t *system_mem2;
volatile bao_uart_t *system_sysram;

void rzt2m_uart_init(volatile R_SCI0_Type * const p_api_ctrl)
{
    
        /*
            system_mem1 = (void*) mem_alloc_map_dev(&cpu()->as, SEC_HYP_GLOBAL, NULL_VA,
                0x80280000, NUM_PAGES(0x1000));
            system_mem2 = (void*) mem_alloc_map_dev(&cpu()->as, SEC_HYP_GLOBAL, NULL_VA,
                0x80281000, NUM_PAGES(0x1000));
            system_sysram = (void*) mem_alloc_map_dev(&cpu()->as, SEC_HYP_GLOBAL, NULL_VA,
                        0x81280000, NUM_PAGES(0x2000));
            system_sysram = (void*) mem_alloc_map_dev(&cpu()->as, SEC_HYP_GLOBAL, NULL_VA,
                        0x81030000, NUM_PAGES(0x10000));
            system_sysram = (void*) mem_alloc_map_dev(&cpu()->as, SEC_HYP_GLOBAL, NULL_VA,
                        0x800A0000, NUM_PAGES(0x10000));
        */

        //clock_init();

        //ioport_open (&g_ioport_ctrl, &g_bsp_pin_cfg);

        baud_setting_t g_uart0_baud_setting =
        {
        .abcse = 0,
        .ccr2_baudrate_bits=2147523342,
        .abcs = 0, .bgdm = 0, .cks = 0, .brr = 155, .mddr = 128, .brme = false
        };

        const sci_uart_extended_cfg_t g_uart0_cfg_extend =
        { .clock = SCI_UART_CLOCK_INT,
        .rx_edge_start = SCI_UART_START_BIT_FALLING_EDGE,
        .noise_cancel = SCI_UART_NOISE_CANCELLATION_DISABLE,
        .rx_fifo_trigger = SCI_UART_RX_FIFO_TRIGGER_MAX,
        .p_baud_setting = &g_uart0_baud_setting,
        .uart_mode = UART_MODE_RS232,
        .ctsrts_en = SCI_UART_CTSRTS_RTS_OUTPUT,
        .sync_bypass = SCI_UART_SYNCHRONIZER_NOT_BYPASS, };

        const uart_cfg_t g_uart0_cfg =
        { 
        .channel = (0x80002000 ==((uint32_t)p_api_ctrl)) ? 4:(((uint32_t) p_api_ctrl>>SCI_NonSecure_mask)&Channel1to3_selector) +((((uint32_t) p_api_ctrl >>SCI_Secure_mask)&0x1)*Channel5_selector),
        .data_bits = UART_DATA_BITS_8,
        .parity = UART_PARITY_OFF,
        .stop_bits = UART_STOP_BITS_1,
        .p_extend = &g_uart0_cfg_extend, 
        };
        renesas_uart_init(p_api_ctrl,&g_uart0_cfg);

}

// Exemplo para o periférico SCI
void R_BSP_ModuleStart_SCI(uint8_t channel)
{

    // Limpa o bit de parada do módulo (habilita o módulo)
    R_SYSC_NS->MSTPCRA &= ~(1U << (8U + channel));

    // Leituras/dummy reads para garantir sincronização de hardware
    (void)R_SYSC_NS->MSTPCRA;
    (void)R_SCI2->RDR;
    (void)R_SCI2->RDR;
    (void)R_SCI2->RDR;
    (void)R_SCI2->RDR;
    (void)R_SCI2->RDR;

}

#define RSELP18 (*(volatile uint8_t*)0x81030C12) // RSELPm: 0x81030C00 + m

void renesas_uart_init (volatile R_SCI0_Type * const p_uart, uart_cfg_t const * const p_cfg)
{
    
    register_protect_disable(BSP_REG_PROTECT_LPC_RESET);
    R_BSP_MODULE_START(FSP_IP_SCI, p_cfg->channel); //previous version we comment to isolate an error when trying to add channel 2 uart
    R_BSP_MODULE_START(FSP_IP_MTU3, 3); //TO WRITE IN TIMER3
    //R_SYSC_NS->MSTPCRC_b.MSTPCRC00 = 0; //JOAO TIMER ENABLE TO WRITE
    RSELP18 |= (1u << 5) | (1u << 4);
    register_protect_enable(BSP_REG_PROTECT_LPC_RESET);

    R_RWP_S->PRCRS = 0x0000A504; // Enable write to GPIO and LPC registers
    // //Confif Ports CH2
    // // P3_5 e P3_6 em Non-safety (RS5=1, RS6=1)
    // RSELP3 |= (1u << 5) | (1u << 6);
    
    // /*Selecionar funções (multiplex) nos nibbles PFC */
    // R_PORT_NSR->PFC_b[3].PFC5 = 5;  // P03_5 -> RXD2
    // R_PORT_NSR->PFC_b[3].PFC6 = 6;  // P03_6 -> TXD2

    // /* Direção (opcional mas recomendado antes do PMC) */
    // R_PORT_NSR->PM_b[3].PM5 = 1;    // P3_5 input (RX)
    // R_PORT_NSR->PM_b[3].PM6 = 0;    // P3_6 output (TX)

    // /* Caminho de entrada do periférico para o RX */
    // //R_PORT_NSR->PIPC_b[3].PIPC5 = 1;

    // /*Colocar os pinos em modo periférico (PMC = 1) */
    // R_PORT_NSR->PMC_b[3].PMC5 = 1;  // P03_5 peripheral
    // R_PORT_NSR->PMC_b[3].PMC6 = 1;  // P03_6 peripheral
    // ////////////////

    //Confif Ports CH4
    // P18_5 e P18_4 em Non-safety (RS4=1, RS5=1)
    // P18_5 -> rxd4
    // P18_4 -> txd4
    // Connectation in Jumper mikroBUS MOSI para P18_5 (rx) e SCK para P18_4 (tx) 
    #if defined(USE_UART4)
        RSELP18 |= (1u << 5) | (1u << 4);
        R_RWP_S->PRCRS = 0x0000A500; // Enable write to GPIO and LPC registers

        R_RWP_NS->PRCRN = 0x0000A504;
        /*Selecionar funções (multiplex) nos nibbles PFC */
        R_PORT_NSR->PFC_b[18].PFC5 = 2;  // P18_5 -> RXD4
        R_PORT_NSR->PFC_b[18].PFC4 = 2;  // P18_4 -> TXD4

        /* Direção (opcional mas recomendado antes do PMC) */
        R_PORT_NSR->PM_b[18].PM5 = 1;    // P3_5 input (RX)
        R_PORT_NSR->PM_b[18].PM4 = 0;    // P3_6 output (TX)

        /* Caminho de entrada do periférico para o RX */
        //R_PORT_NSR->PIPC_b[18].PIPC5 = 1;

        /*Colocar os pinos em modo periférico (PMC = 1) */
        R_PORT_NSR->PMC_b[18].PMC5 = 1;  // P18_5 peripheral
        R_PORT_NSR->PMC_b[18].PMC4 = 1;  // P18_4 peripheral
        R_RWP_NS->PRCRN = 0x0000A500;
    #endif
    ////////////////



    p_uart->CCR0 = SCI_UART_CCR0_DEFAULT_VALUE;
    FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.RE, 0);
    FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.TE, 0);
    
    p_uart->CCR1 = SCI_UART_CCR1_DEFAULT_VALUE;
    p_uart->CCR2 = SCI_UART_CCR2_DEFAULT_VALUE;
    p_uart->CCR3 = SCI_UART_CCR3_DEFAULT_VALUE;
    p_uart->CCR4 = SCI_UART_CCR4_DEFAULT_VALUE;

    /* Set the UART configuration settings provided in ::uart_cfg_t and ::sci_uart_extended_cfg_t. */
    uart_config_set(p_uart, p_cfg);

    uint32_t ccr0 = SCI_UART_CCR0_IDSEL_MASK;

    //enable interrupt to recieve
    //ccr0 |= ( SCI_UART_CCR0_TIE_MASK );
    ccr0 |= ( SCI_UART_CCR0_RIE_MASK);

    //Enable to recieve
    ccr0 |= ( SCI_UART_CCR0_RE_MASK); 

    //Enable to transmit
    ccr0 |= SCI_UART_CCR0_TE_MASK;
    p_uart->CCR0 = ccr0; 

    FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.RE, 1);
    FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.TE, 1);

}

void uart_read (volatile R_SCI0_Type * const p_uart, uint32_t * const p_dest, uint32_t const bytes)
{
    //
    *p_dest = p_uart->RDR_b.RDAT;
    FSP_HARDWARE_REGISTER_WAIT(p_uart->FRSR_b.DR,0);

    /* @TODO Ir buscar como ler da porta serie  a funçao sci_uart_rxi_common */
}

uint32_t rzt2m_uart_getc(volatile R_SCI0_Type * p_uart){
    uint32_t result;
    uart_read(p_uart,&result,1);
    return (uint32_t)result;
}

void uart_write (volatile R_SCI0_Type * const p_uart, uint8_t const * const p_src)
{
    FSP_HARDWARE_REGISTER_WAIT(p_uart->CSR_b.TEND, 1);  // Wait for the data to be transmitted

    p_uart->TDR_b.TDAT = *(p_src); //Print the data
    
   // FSP_HARDWARE_REGISTER_WAIT(p_uart->CSR_b.TDRE, 1);  // Wait for the data to be transmitted

}

void renesas_set_baud_rate(volatile R_SCI0_Type * const p_uart, void const * const p_baud_setting)
{ 

        /* Disables transmitter and receiver. This terminates any in-progress transmission. */
        p_uart->CCR0 &= (uint32_t) ~(SCI_UART_CCR0_TE_MASK | SCI_UART_CCR0_TIE_MASK | SCI_UART_CCR0_TEIE_MASK |
                                            SCI_UART_CCR0_RE_MASK | SCI_UART_CCR0_RIE_MASK); 
        FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.RE, 0);
        FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.TE, 0);

        /* Apply new baud rate register settings. */
        r_sci_uart_baud_set(p_uart, p_baud_setting);
        
}

void rzt2m_uart_putc(volatile R_SCI0_Type * ptr_uart,uint8_t c)
{
    uart_write (ptr_uart, &c);
}

void uart_puts(volatile R_SCI0_Type * ptr_uart,const char *s)
{
    while (*s)
	{
		rzt2m_uart_putc(ptr_uart,*s++);
	}
}


void rzt2m_uart_disable(volatile R_SCI0_Type * const p_uart)
{
        /* Disables transmitter and receiver. This terminates any in-progress transmission. */
        p_uart->CCR0 &= (uint32_t) ~(SCI_UART_CCR0_TE_MASK | SCI_UART_CCR0_TIE_MASK | SCI_UART_CCR0_TEIE_MASK |
                                            SCI_UART_CCR0_RE_MASK | SCI_UART_CCR0_RIE_MASK); 
        FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.RE, 0);
        FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.TE, 0);
}

void rzt2m_uart_enable(volatile R_SCI0_Type * const p_uart)
{
    uint32_t mask_enable = 0;

    /* Enable receive. */
    mask_enable |= (SCI_UART_CCR0_RE_MASK );
    /* Enable transmit. */
    mask_enable |= SCI_UART_CCR0_TE_MASK; 

    //enable interrupt to recieve
    //mask_enable |= ( SCI_UART_CCR0_TIE_MASK );
    //mask_enable |= ( SCI_UART_CCR0_RIE_MASK);
    
    p_uart->CCR0 |= mask_enable; 
    FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.RE, 1);
    FSP_HARDWARE_REGISTER_WAIT(p_uart->CCR0_b.TE, 1);

}

void rzt2m_uart_enable_rxirq(volatile R_SCI0_Type * const p_uart)
{
    uint32_t mask = 0;

    /* Enable RX interrupt */
    mask = (SCI_UART_CCR0_RIE_MASK);
    
    p_uart->CCR0 |= mask;
}

#define SCI_UART_CFCLR_ORERC_MASK             (0x01000000)
#define SCI_UART_CFCLR_FERC_MASK              (0x10000000)
#define SCI_UART_CFCLE_PERC_MASK              (0x08000000)
#define SCI_UART_RCVR_ERRCLR_MASK             (SCI_UART_CFCLR_ORERC_MASK | SCI_UART_CFCLR_FERC_MASK | \
                                               SCI_UART_CFCLE_PERC_MASK)


void renesas_clear_uart(volatile  R_SCI0_Type * const p_api_ctrl)
{
        p_api_ctrl->CFCLR |= (uint32_t) (SCI_UART_RCVR_ERRCLR_MASK);
        volatile uint32_t dummy = p_api_ctrl->CSR;


}