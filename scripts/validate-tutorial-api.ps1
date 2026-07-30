# validate-tutorial-api.ps1
# Scans docs/web-tutorial/*.html for function calls not declared in any .h file
# Usage:  powershell -File scripts\validate-tutorial-api.ps1 [-HtmlDir docs\web-tutorial] [-Strict]
# Note:   This is a heuristic tool — expect ~80% false positives (local variables,
#         callback names, enum values, etc.). Review output manually before making changes.
param(
    [string]$HtmlDir = 'docs\web-tutorial',
    [switch]$Strict
)

$ErrorActionPreference = 'Continue'

# ===== Step 1: Build set of real function names from headers =====
Write-Host '[1/3] Extracting real functions from headers...'
$realFuncs = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)

$headerDirs = @('User\SimpleHAL', 'User\Lib')
foreach ($dir in $headerDirs) {
    if (-not (Test-Path $dir)) { continue }
    Get-ChildItem $dir -Filter '*.h' -Recurse -ErrorAction SilentlyContinue | ForEach-Object {
        $txt = Get-Content $_.FullName -Raw -Encoding UTF8 -ErrorAction SilentlyContinue
        if (-not $txt) { return }
        $txt = $txt -replace '//.*$', ''
        # void/int/uintX_t/float/double/char/bool Name(
        $pat1 = '\b(?:void|int|uint\d+_t|int\d+_t|u?int(?:8|16|32|64)_t|float|double|char\*?|bool|uint8_t|uint16_t|uint32_t)\s+(\w{3,})\s*\('
        [regex]::Matches($txt, $pat1) | ForEach-Object { $null = $realFuncs.Add($_.Groups[1].Value) }
        # Status_Type Name(
        $pat2 = '\b([A-Z][a-zA-Z0-9_]*_Status)\s+(\w{3,})\s*\('
        [regex]::Matches($txt, $pat2) | ForEach-Object { $null = $realFuncs.Add($_.Groups[2].Value) }
        # Name* constructors: HandleType* Name(
        $pat3 = '\b([A-Z][a-zA-Z0-9_]*_Handle)\s*\*\s*(\w{3,})\s*\('
        [regex]::Matches($txt, $pat3) | ForEach-Object { $null = $realFuncs.Add($_.Groups[2].Value) }
    }
}
Write-Host "  Found $($realFuncs.Count) real function declarations"

# ===== Step 2: Build allowlist =====
$ok = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
@(
    # C stdlib
    'printf','sprintf','snprintf','fprintf','puts','strcmp','strncmp','strstr','strchr','strlen','strcpy','strncpy','strcat',
    'memset','memcpy','memcmp','memmove','atoi','atol','atof','atod','abs','labs','min','max',
    'sizeof','offsetof','typeof','malloc','free','calloc','realloc','assert','exit',
    'isalpha','isdigit','isspace','toupper','tolower',
    # ARM CMSIS
    '__disable_irq','__enable_irq','__NOP','__WFI','__WFE','NVIC_SystemReset','NVIC_EnableIRQ','NVIC_DisableIRQ','NVIC_SetPriority','SysTick_Config',
    # System
    'SystemCoreClockUpdate','SystemInit',
    # SimpleHAL builtins
    'pinMode','digitalWrite','digitalRead','digitalToggle','attachInterrupt','detachInterrupt',
    'millis','micros','random','randomSeed',
    'Delay_Ms','Delay_Us','Timer_Init','Start_Timer','Is_Timer_Expired','Timer_t',
    'USART_Print','USART_Println','USART_PrintHex','USART_PrintNum','USART_PrintFloat','USART_Printf',
    'USART_WriteByte','USART_Write','USART_Read','USART_Available',
    'USART1_Init','USART2_Init','USART1_Available','USART2_Available','USART1_Read','USART2_Read',
    'I2C_SimpleInit','SPI_SimpleInit','USART_SimpleInit','ADC_SimpleInit',
    'IWDG_Feed','I2C_Init','SPI_Init','ADC_Read',
    'PWM_SetDutyCycle','PWM_Start','PWM_Stop','PWM_Init',
    'OLED_Print','OLED_Clear','OLED_PrintFloat','OLED_DrawStringAlign',
    'OLED_Update','OLED_SetFont','OLED_Display','OLED_Init',
    'LCD_Print','LCD_Clear','LCD_Init','LCD_SetCursor',
    # STD Peripheral
    'FLASH_Unlock','FLASH_Lock','FLASH_EraseOptionBytes','FLASH_ReadOutProtection',
    'GPIO_Init','GPIO_SetBits','GPIO_ResetBits','GPIO_ReadInputDataBit','GPIO_WriteBit',
    'RCC_APB2PeriphClockCmd','RCC_APB1PeriphClockCmd',
    'USART_SendData','USART_ReceiveData','USART_GetFlagStatus','USART_ITConfig',
    'PWR_PVDLevelConfig','PWR_PVDCmd','PWR_GetFlagStatus','PWR_ClearFlag','PWR_BackupAccessCmd',
    'PWR_EnterStandbyMode','PWR_Standby','PWR_StandbyUntilInterrupt','PWR_ConfigureAWU',
    'IWDG_WriteAccessCmd','IWDG_SetPrescaler','IWDG_SetReload','IWDG_ReloadCounter','IWDG_Enable',
    'EXTI_Init','EXTI_GetITStatus','EXTI_ClearITPendingBit',
    'TIM_Cmd','TIM_SetCounter','TIM_GetCounter','TIM_PrescalerConfig','TIM_ITConfig','TIM_ARRPreloadConfig',
    'TIM_OC1Init','TIM_OC2Init','TIM_OC3Init','TIM_OC4Init','TIM_CtrlPWMOutputs','TIM_SetCompare1',
    'DMA_Init','DMA_Cmd','ADC_Init','ADC_Cmd','ADC_GetConversionValue','ADC_SoftwareStartConvCmd',
    'I2C_Cmd','I2C_GenerateSTART','I2C_SendData','I2C_ReceiveData',
    'SPI_Cmd','SPI_I2S_SendData','SPI_I2S_ReceiveData','SPI_I2S_GetFlagStatus',
    'WWDG_Init','WWDG_Cmd','WWDG_SetPrescaler','WWDG_SetWindowValue','WWDG_SetCounter',
    'IWDG_Init','DBGMCU_GetCHIPID',
    'Flash_SaveConfig','Flash_LoadConfig',
    'ADC_GetBatteryPercent','ADC_GetBatteryMillivolt',
    # Package / enum helpers
    'PACKAGE_TSSOP20','PACKAGE_SOP8','PACKAGE_SOP16','PACKAGE_QFN20',
    'CH32V003_PACKAGE','CH32V003_IS_SOP8',
    # Constants often in code blocks
    'M_PI','true','false','NULL','HIGH','LOW','ENABLE','DISABLE','RESET','SET',
    'INPUT','OUTPUT','INPUT_PULLUP','INPUT_PULLDOWN',
    'RISING','FALLING','CHANGE',
    'BAUD_115200','BAUD_9600','BAUD_19200','BAUD_38400','BAUD_57600',
    'PIN_MODE_INPUT','PIN_MODE_OUTPUT','PIN_MODE_INPUT_PULLUP','PIN_MODE_INPUT_PULLDOWN',
    'I2C_SPEED_100K','I2C_SPEED_400K','I2C_SPEED_STANDARD','I2C_SPEED_FAST',
    'I2C_PINS_DEFAULT','I2C_PINS_REMAP','I2C_PINS_PARTIAL_REMAP',
    'SPI_SPEED_4MHZ','SPI_SPEED_2MHZ','SPI_SPEED_1MHZ','SPI_12MHZ',
    'SPI_MODE_0','SPI_MODE_1','SPI_MODE_2','SPI_MODE_3',
    'SPI_PINS_DEFAULT','SPI_PINS_FULL_REMAP',
    'USART_PINS_DEFAULT','USART_PINS_SWAP','USART_PINS_FULL_REMAP',
    'OLED_128x64','OLED_128x32','OLED_64x48',
    'ALIGN_LEFT','ALIGN_CENTER','ALIGN_RIGHT',
    'FONT_8x16','FONT_6x8',
    'DHT_OK','DHT_ERROR_TIMEOUT','DHT_ERROR_CHECKSUM','DHT_ERROR_NOT_INIT',
    'DHT_TYPE_DHT11','DHT_TYPE_DHT22',
    'DHT_StatusStr',
    'PWM1_CH1','PWM1_CH2','PWM1_CH3','PWM1_CH4','PWM2_CH1','PWM2_CH2','PWM2_CH3','PWM2_CH4',
    'boolean',
    'LCD_16x2','LCD_20x4',
    'DS3231_MONDAY','DS3231_TUESDAY','DS3231_WEDNESDAY',
    'STEPPER_HALF_STEP','STEPPER_FULL_STEP',
    'RELAY_ACTIVE_HIGH','RELAY_ACTIVE_LOW',
    'BUZZER_ACTIVE_HIGH','BUZZER_ACTIVE_LOW'
) | ForEach-Object { $null = $ok.Add($_) }

# HTML tags / CSS noise
$noise = [System.Collections.Generic.HashSet[string]]::new([StringComparer]::OrdinalIgnoreCase)
@(
    'class','style','id','href','src','type','lang','rel','span','div',
    'code','pre','html','head','body','meta','link','title','nav','section',
    'table','tr','td','th','thead','tbody','tfoot','h1','h2','h3','h4','h5','h6',
    'p','a','ul','ol','li','br','hr','img','input','button','form','label',
    'select','option','script','strong','em','small','sub','sup','blockquote',
    'header','footer','main','article','aside','details','summary',
    'figure','figcaption','video','audio','source','canvas','svg','path',
    'circle','rect','line','polygon','polyline','g','defs','opencode'
) | ForEach-Object { $null = $noise.Add($_) }

# ===== Step 3: Scan HTML files =====
Write-Host "[2/3] Scanning HTML code blocks..."
$vlist = New-Object 'System.Collections.Generic.List[PSCustomObject]'
$seen = [System.Collections.Generic.HashSet[string]]::new()

$htmlFiles = Get-ChildItem $HtmlDir -Filter '*.html' -ErrorAction SilentlyContinue
$total = $htmlFiles.Count

foreach ($file in $htmlFiles) {
    $text = Get-Content $file.FullName -Raw -Encoding UTF8 -ErrorAction SilentlyContinue
    if (-not $text) { continue }

    # Extract code blocks
    $cb = [regex]::Matches($text, '<code[^>]*?>(.*?)</code>', [System.Text.RegularExpressions.RegexOptions]::Singleline)
    foreach ($b in $cb) {
        $codeBlock = $b.Groups[1].Value
        $codeBlock = $codeBlock -replace '&lt;', '<'
        $codeBlock = $codeBlock -replace '&gt;', '>'
        $codeBlock = $codeBlock -replace '&amp;', '&'
        $codeBlock = $codeBlock -replace '&quot;', '"'

        $calls = [regex]::Matches($codeBlock, '\b([A-Z][a-zA-Z0-9_]{2,}|[a-z][a-zA-Z0-9_]*_[a-zA-Z0-9_]{2,})\.?\s*\(')
        foreach ($c in $calls) {
            $fn = $c.Groups[1].Value
            if ($ok.Contains($fn)) { continue }
            if ($realFuncs.Contains($fn)) { continue }
            if ($noise.Contains($fn)) { continue }
            if ($fn -cmatch '^[a-z]+$') { continue }

            $key = $file.Name + '|' + $fn
            if ($seen.Contains($key)) { continue }
            $null = $seen.Add($key)

            $vlist.Add([PSCustomObject]@{ File = $file.Name; Function = $fn })
        }
    }
}

# ===== Report =====
Write-Host "[3/3] Results:"

if ($vlist.Count -eq 0) {
    Write-Host '  PASSED - No fabricated API names found!'
    Write-Host ''
    exit 0
}

Write-Host "  FAILED - $($vlist.Count) fabricated function name(s) across $((($vlist | Group-Object File).Count)) file(s):"
Write-Host ''

$byFile = $vlist | Group-Object File | Sort-Object Name
foreach ($g in $byFile) {
    Write-Host "  -- $($g.Name) --"
    foreach ($v in ($g.Group | Sort-Object Function)) {
        $line = '      ' + $v.Function
        Write-Host $line -ForegroundColor Red
    }
}

Write-Host ''
Write-Host '  Note: many matches may be false positives (local vars, callbacks, enums).'
Write-Host '  Review each manually. Use -Strict to fail CI on violations.'
Write-Host ''
if ($Strict -and $vlist.Count -gt 0) {
    exit 1
}
exit 0
