; Timer 1 interrupt vector. Executes 15,625 times per second (every 1024 cycles - so it must not take longer than this).
; Here where we do all the things which must be done at a particular time.
; The LED output, audio output and switch reading code is mixed up so that we don't need to explicitly wait for the SPI transmits and the switch column selection.

;SIGNAL(TIMER1_OVF_vect)
;{
.global __vector_13              ; 7
__vector_13:  ; TIMER1_OVF_vect
  push r31                       ; 2
  push r30                       ; 2
  push r29                       ; 2
  push r28                       ; 2
  push r27                       ; 2
  push r26                       ; 2
  push r25                       ; 2
  push r24                       ; 2
  push r1                        ; 2
  push r0                        ; 2
  in r0, 0x3f                    ; 1
  push r0                        ; 2

;    // Output the LED data
;    uint16_t leds = 0;
;    uint16_t rowRegister = 1 << lineInFrame;
;    for (uint8_t column = 0; column < 0x10; ++column)
;        if ((lineBuffer[column]--) > 0)
;            leds |= (1 << column);
;    SPDR = rowRegister >> 8;
;    // delay at least 16 cycles
;    SPDR = rowRegister;
;    // delay at least 16 cycles
;    SPDR = leds >> 8;
;    // delay at least 16 cycles
;    SPDR = leds;
;    // delay at least 16 cycles

  ldi r31, hi8(rowRegisterData)  ; 1  hi8(Z) = hi8(rowRegisterData)
  ldi r30, lo8(rowRegisterData)  ; 1  lo8(Z) = lo8(rowRegisterData)
  add r30, r4                    ; 1  lo8(Z) = lo8(rowRegisterData) + lineInFrame
  add r30, r4                    ; 1  lo8(Z) = lo8(rowRegisterData) + 2*lineInFrame
  lpm r23, Z+                    ; 3  r23 = lo8(rowRegisterData[lineInFrame])
  out 0x2e, r23                  ; 1  SPDR = r23  (Output shift register data for lower rows. Need to wait at least 16 cycles before loading next byte.)
  lpm r23, Z                     ; 2  r23 = hi8(rowRegisterData[lineInFrame])

  ldi r30, lo8(lineBuffer)       ; 1  lo8(Z) = lo8(lineBuffer) = 0x70
  ldi r31, hi8(lineBuffer)       ; 1  hi8(Z) = hi8(lineBuffer) = 0x02

  eor r25, r25                   ; 1  r25 = 0

.macro illuminate column
  ldd r24, Z+\column             ; 2    r24 = lineBuffer[column]
  cp r2, r24                     ; 1    if (lineBuffer[column] <= 0)
  brlt 1f                        ; 2 1
  ori r25, 1<<(\column&7)        ; 0 1      r25 |= (1<<column)
1:
  subi r24, 1                    ; 1    --r24
  std Z+\column, r24             ; 2    --lineBuffer[column]
.endm

  illuminate 0x08                ; 8
  illuminate 0x09                ; 8
  illuminate 0x0a                ; 8
  illuminate 0x0b                ; 8

  out 0x2e, r23                  ; 1  SPDR = r23  (Output shift register data for upper rows.)

  illuminate 0x0c                ; 8
  illuminate 0x0d                ; 8
  illuminate 0x0e                ; 8
  illuminate 0x0f                ; 8

  out 0x2e, r25                  ; 1  SPDR = r25  (Output shift register data for right columns.)
  eor r25, r25                   ; 1

  illuminate 0x00                ; 8
  illuminate 0x01                ; 8
  illuminate 0x02                ; 8
  illuminate 0x03                ; 8
  illuminate 0x04                ; 8
  illuminate 0x05                ; 8
  illuminate 0x06                ; 8
  illuminate 0x07                ; 8

  out 0x2e, r25                  ; 1  SPDR = r25  (Output shift register data for left columns.)

;    PORTB = 0x11;  // output shift register reset low
;    PORTB = 0x15;  // output shift register reset high

TODO: wait until...


  ; That last shift register should be done by now
  ldi r25, 0x11                  ; 1
  out 0x05, r25                  ; 1   PORTB = 0x11  (output shift register reset low)
  ldi r25, 0x15                  ; 1
  out 0x05, r25                  ; 1   PORTB = 0x15  (output shift register reset high)


;    OCR2B = (total >> 12) + 0x80;


;    // Update water
;    if (lightBuffers[lightPage][switchInFrame] != 0 && sampleInLine == beatInPattern && (frameInBeat>>8) < 4) {
;        // Put energy into the water as we play
;        waterBuffers[waterPage^1][switchInFrame] = 16;
;    }
;    else {
;        // Allow the energy to spread out
;        uint16_t total = 0;
;        if (sampleInLine > 0)
;            total = waterBuffers[waterPage][switchInFrame - 1];
;        if (sampleInLine < 0x0f)
;            total += waterBuffers[waterPage][switchInFrame + 1];
;        if (lineInFrame > 0)
;            total += waterBuffers[waterPage][switchInFrame - 0x10];
;        if (lineInFrame < 0x0f)
;            total += waterBuffers[waterPage][switchInFrame + 0x10];
;        total >>= 1;
;        total -= waterBuffers[waterPage^1][switchInFrame];
;        total -= (total >> 8);
;        waterBuffers[waterPage^1][switchInFrame] = total;
;    }

updateWater:
  mov r31, r10                   ; 1 1 1 1  hi8(Z) = lightPage
  ld r25, Z                      ; 2 2 2 2  Z = lightBuffers[lightPage][switchInFrame]
  mov r31, r15                   ; 1 1 1 1  hi8(Z) = waterPage
  tst r25                        ; 1 1 1 1  if (!lightOn)
  breq waveEquation              ; 2 1 1 1      goto waveEquation
  cp r3, r9                      ; 0 1 1 1  if (sampleInLine != beatInPattern)
  brne waveEquation              ; 0 2 1 1      goto waveEquation
  mov r24, r6                    ; 0 0 1 1  r24 = frameInBeat >> 8
  cpi r24, 4                     ; 0 0 1 1  if ((frameInBeat >> 8) >= 4)
  brsh waveEquation              ; 0 0 2 1      goto waveEquation
  ldi r23, 16                    ; 0 0 0 1  r23 = 16
  eor r31, r18                   ; 0 0 0 1  hi8(Z) ^= 1
  rjmp storeWaterLevel           ; 0 0 0 2  goto storeWaterLevel
waveEquation:
  ldi r24, 0                     ; 1
  ldi r23, 0                     ; 1        r24:r23 = 0  (total = 0)
  subi r30, 1                    ; 1        lo8(Z) = switchInFrame - 1
  cp r3, r2                      ; 1        if (sampleInLine != 0)
  breq doneLeft                  ; 2 1
  ld r23, Z                      ; 0 2          r23 = waterBuffers[waterPage][switchInFrame - 1]
  sbrc r23, 7                    ; 0 1 2
  subi r24, 1                    ; 0 1 0        r24:r23 = waterBuffers[waterPage][switchInFrame - 1]  (sign extend)
doneLeft:
  mov r21, r3                    ; 1
  subi r30, -2                   ; 1        lo8(Z) = switchInFrame + 1
  cpi r21, 15                    ; 1        if (sampleInLine != 0x0f)
  breq doneRight                 ; 2 1
  ld r25, Z                      ; 0 2          r25 = waterBuffers[waterPage][switchInFrame + 1]
  muls r25, r18                  ; 0 2          r1:r0 = waterBuffers[waterPage][switchInFrame + 1]*1 (sign extend)
  add r23, r0                    ; 0 1
  adc r24, r1                    ; 0 1          r24:r23 += waterBuffers[waterPage][switchInFrame + 1]
doneRight:
  mov r20, r4                    ; 1
  subi r30, 0x11                 ; 1        lo8(Z) = switchInFrame - 0x10
  cpi r20, 0                     ; 1        if (lineInFrame != 0)
  breq doneUp                    ; 2 1
  ld r25, Z                      ; 0 2          r25 = waterBuffers[waterPage][switchInFrame - 0x10]
  muls r25, r18                  ; 0 2          r1:r0 = waterBuffers[waterPage][switchInFrame - 0x10]*1 (sign extend)
  add r23, r0                    ; 0 1
  adc r24, r1                    ; 0 1          r24:r23 += waterBuffers[waterPage][switchInFrame - 0x10]
doneUp:
  subi r30, -0x20                ; 1        lo8(Z) = switchInFrame + 0x10
  cpi r20, 15                    ; 1        if (lineInFrame != 0x0f)
  breq doneDown                  ; 2 1
  ld r25, Z                      ; 0 2          r25 = waterBuffers[waterPage][switchInFrame + 0x10]
  muls r25, r18                  ; 0 2          r1:r0 = waterBuffers[waterPage][switchInFrame + 0x10]*1 (sign extend)
  add r23, r0                    ; 0 1
  adc r24, r1                    ; 0 1          r24:r23 += waterBuffers[waterPage][switchInFrame - 0x10]
doneDown:
  subi r30, 0x10                 ; 1        lo8(Z) = switchInFrame
  asr r24                        ; 1
  ror r23                        ; 1        r24:r23 >>= 1  (total >>= 1)
  eor r31, r18                   ; 1        hi8(Z) ^= 1
  ld r25, Z                      ; 2        r25 = waterBuffers[1-waterPage][switchInFrame]
  muls r25, r18                  ; 2        r1:r0 = waterBuffers[1-waterPage][switchInFrame]*1 (sign extend)
  sub r23, r0                    ; 1
  sbc r24, r1                    ; 2        r24:r23 -= waterBuffers[1-waterPage][switchInFrame]
  sub r23, r24                   ; 2        total -= (total >> 8)
storeWaterLevel:
  st Z, r23                      ; 2        waterBuffers[1-waterPage][switchInFrame] = total

;    // Update counters
;    ++switchInFrame;
;    ++sampleInLine;
;    if (sampleInLine == 0x10) {
;        // Per line code
;        sampleInLine = 0;
;        ++lineInFrame;
;        if (lineInFrame == 0x10) {
;            // Per frame code
;            lineInFrame = 0;
;            if (outputSyncPulseActive) {
;                outputSyncPulseActive = false;
;                PORTD = 0x91;
;            }
;            waterPage ^= 1;
;            if (!waitingForSync) {
;                frameInBeat += 0x100;
;                if (frameInBeat >= framesPerBeat) {
;                    // Per beat code
;                    frameInBeat -= framesPerBeat;
;                    framesPerBeat = nextFramesPerBeat;
;                    ++beatInPattern;
;                    beatInPattern &= 0x0f;
;                    if (beatInPattern == beatsPerPattern) {
;                        beatInPattern = 0x10;
;                        waitingForSync = true;
;                        outputSyncPulseActive = true;
;                        PORTD = 0x95;
;                        if (lifeMode)
;                            lightPage ^= 1;
;                        else {
;                            ++patternInLoop;
;                            if (patternInLoop == patternsPerLoop)
;                                patternInLoop = 0;
;                        }
;                    }
;                }
;            }
;        }
;        if (waitingForSync)
;            if ((PINB & 1) != 0)
;                receivedSync = true;
;        if (lineInFrame == 0 && receivedSync) {
;            receivedSync = false;
;            waitingForSync = false;
;            beatInPattern = 0;
;        }
;        // Update line buffer
;        if (patternMode)
;            for (uint8_t column = 0; column < 0x10; ++column) {
;                uint8_t led = (lineInFrame<<4) | column;
;                uint8_t value = waterBuffers[waterPage][led] >> 1;
;                if (lightBuffers[lightPage][led]) {
;                    if (column == beatInPattern)
;                        value = 0x0f;
;                    else
;                        value += 10;
;                }
;                lineBuffer[column] = value;
;            }
;        else
;            for (uint8_t column = 0; column < 0x10; ++column)
;                lineBuffer[column] = frameBuffer[(lineInFrame<<4) | column];
;    }

  inc r11                        ; 1 1 1 1 1 1 1 1              ++switchInframe
  inc r3                         ; 1 1 1 1 1 1 1 1              ++sampleInLine
  cp r3, r19                     ; 1 1 1 1 1 1 1 1              if (sampleInLine == 0x10)
  breq .+2                       ; 1 2 2 2 2 2 2 2
  rjmp checkDecay                ; 2 0 0 0 0 0 0 0
  mov r3, r2                     ; 0 1 1 1 1 1 1 1                  sampleInLine = 0
  inc r4                         ; 0 1 1 1 1 1 1 1                  ++lineInFrame
  cp r4, r19                     ; 0 1 1 1 1 1 1 1                  if (lineInFrame == 0x10)
  brne checkWaitingForSync       ; 0 2 1 1 1 1 1 1
  mov r4, r2                     ; 0 0 1 1 1 1 1 1                      lineInFrame = 0
  sbrs r17, 3                    ; 0 0 1 1 1 1 1 1                      if (flags & 8)  (outputSyncPulseActive)                       2
  rjmp outputSyncPulseNotActive  ; 0 0 2 2 2 2 2 2                                                                                    0
  andi r17, 0xf7                 ; 0 0 0 0 0 0 0 0                          flags &= 0xf7  (outputSyncPulseActive = false)            1
  ldi r31, 0x91                  ; 0 0 0 0 0 0 0 0                          r31 = 0x91                                                1
  out 0x0b, r31                  ; 0 0 0 0 0 0 0 0                          PORTD = 0x91                                              1
outputSyncPulseNotActive:
  eor r15, r18                   ; 0 0 1 1 1 1 1 1                      waterPage ^= 1
  sbrc r17, 0                    ; 0 0 1 2 2 2 2 2                      if (!waitingForSync)
  rjmp waitingForSync            ; 0 0 2 0 0 0 0 0
  inc r6                         ; 0 0 0 1 1 1 1 1                          ++hi8(frameInBeat)
  cp r5, r7                      ; 0 0 0 1 1 1 1 1
  cpc r6, r8                     ; 0 0 0 1 1 1 1 1                          if (frameInBeat >= framesPerBeat)
  brlt checkReceivedSync         ; 0 0 0 2 1 1 1 1
  sub r5, r7                     ; 0 0 0 0 1 1 1 1
  sbc r6, r8                     ; 0 0 0 0 1 1 1 1                              frameInBeat -= framesPerBeat
  lds r7, nextFramesPerBeatLow   ; 0 0 0 0 2 2 2 2
  lds r8, nextFramesPerBeatHigh  ; 0 0 0 0 2 2 2 2                              framesPerBeat = nextFramesPerBeat
  inc r9                         ; 0 0 0 0 1 1 1 1                              ++beatInPattern
  ldi r31, 0x0f                  ; 0 0 0 0 1 1 1 1
  and r9, r31                    ; 0 0 0 0 1 1 1 1                              beatInPattern &= 0x0f;
  cp r9, r14                     ; 0 0 0 0 1 1 1 1                              if (beatInPattern == beatsPerPattern)
  brne checkReceivedSync         ; 0 0 0 0 2 1 1 1
  mov r9, r19                    ; 0 0 0 0 0 1 1 1                                  beatInPattern = 0x10
  ori r17, 9                     ; 0 0 0 0 0 1 1 1                                  flags |= 9  (waitingForSync = true, outputSyncPulseActive = true)
  ldi r31, 0x95                  ; 0 0 0 0 0 1 1 1                                  r31 = 0x95
  out 0x0b, r31                  ; 0 0 0 0 0 1 1 1                                  PORTD = 0x95
  sbrs r17, 1                    ; 0 0 0 0 0 1 2 1                                  if ((flags & 2) != 0)  (lifeMode != 0)
  rjmp notLifeMode               ; 0 0 0 0 0 2 0 2
  eor r10, r18                   ; 0 0 0 0 0 0 1 0                                      lightPage ^= 1
  rjmp waitingForSync            ; 0 0 0 0 0 0 2 0                                  else
notLifeMode:
  lds r31, patternInLoop         ; 0 0 0 0 0 2 0 2
  inc r31                        ; 0 0 0 0 0 1 0 1                                      ++patternInLoop
  lds r30, patternsPerLoop       ; 0 0 0 0 0 2 0 2
  cp r31, r30                    ; 0 0 0 0 0 1 0 1                                      if (patternInLoop == patternsPerLoop)
  brne noRestartLoop             ; 0 0 0 0 0 2 0 1
  ldi r31, 0                     ; 0 0 0 0 0 0 0 1                                          patternInLoop = 0
noRestartLoop:
  sts patternInLoop, r31         ; 0 0 0 0 0 2 0 2
  rjmp waitingForSync            ; 0 0 0 0 0 2 0 2
checkWaitingForSync:
  sbrs r17, 0                    ; 1 2 2 2 2 2                      if (flags & 1)  (waitingForSync)
  rjmp checkReceivedSync         ; 2 0 0 0 0 0
waitingForSync:
  in r31, 0x03                   ; 0 1 1 1 1 1                          r31 = PINB
  sbrs r31, 0                    ; 0 1 2 1 2 1                          if ((PINB & 1) != 0)
  rjmp checkReceivedSync         ; 0 2 0 2 0 2
  ori r17, 4                     ; 0 0 1 0 1 0                              flags |= 4  (receivedSync = true)
  rjmp receivedSync              ; 0 0 2 0 2 0
checkReceivedSync:
  sbrs r17, 2                    ; 0 1 0 2 0 2                      if ((flags & 4) != 0)  (receivedSync)
  rjmp updateLineBuffer          ; 0 2 0 0 0 0
receivedSync:
  cp r4, r2                      ; 0 0 1 1 1 1                          if (lineInFrame == 0)
  brne updateLineBuffer          ; 0 0 2 2 1 1
  andi r17, 0xfa                 ; 0 0 0 0 1 1                              flags &= 0xfa  (waitingForSync = false, receivedSync = false)
  mov r9, r2                     ; 0 0 0 0 1 1                              beatInPattern = 0

updateLineBuffer:
  sbrs r17, 6                    ; 1 2                              if ((flags & 0x40) != 0)  (patternMode != 0)
  rjmp frameBufferMode           ; 2 0

  mov r30, r4                    ; 0 1                                  lo8(Z) = lineInFrame
  swap r30                       ; 0 1                                  lo8(Z) = lineInFrame<<4
  mov r31, r15                   ; 0 1                                  hi8(Z) = waterPage
  mov r29, r10                   ; 0 1                                  hi8(Y) = lightPage
  mov r28, r30                   ; 0 1                                  lo8(Y) = lineInFrame<<4

.macro initRow column          ; 9
  ldd r25, Z+\column           ; 2 2  r25 = waterBuffers[waterPage][(lineInFrame << 4) | column]
  ldd r24, Y+\column           ; 2 2  r24 = lightBuffers[lightPage][(lineInFrame << 4) | column]
  lsr r25                      ; 1 1  r25 = waterBuffers[waterPage][(lineInFrame << 4) | column] >> 1
  sbrc r24, 0                  ; 1 2  if (lightBuffers[lightPage][(lineInFrame << 4) | column])
  subi r25, -10                ; 1 0      r25 += 10
  sts lineBuffer+\column, r25  ; 2 2  lineBuffer[column] = r25
.endm

  unroll initRow                 ; 0 144

  add r28, r9                    ; 0 1    lo8(Y) = (lineInFrame<<4) | beatInPattern
  ld r24, Y                      ; 0 2    r24 = lightBuffers[lightPage][(lineInFrame << 4) | column]
  tst r24                        ; 0 1    if (lightBuffers[lightPage][(lineInFrame << 4) | column])
  breq checkDecay                ; 0 2 1
  ldi r31, hi8(lineBuffer)       ; 0 0 1
  ldi r30, lo8(lineBuffer)       ; 0 0 1
  add r30, r9                    ; 0 0 1
  ldi r25, 0x0f                  ; 0 0 1
  st Z, r25                      ; 0 0 2

  rjmp checkDecay                ; 0 2
frameBufferMode:                 ;                              else
  ldi r26, 10                    ; 1 0                              r26 = 10
  ldi r27, 0                     ; 1 0                              r27 = 0
  ldi r31, hi8(frameBuffer)      ; 1 0                              hi8(Z) = hi8(frameBuffer)
  mov r30, r4                    ; 1 0                              lo8(Z) = lineInFrame
  swap r30                       ; 1 0                              lo8(Z) = lineInFrame << 4

.macro initFromFrameBuffer column  ; 4
  ld r24, Z+                   ; 2  r24 = frameBuffer[(lineInFrame << 4) | column]
  sts lineBuffer+\column, r24  ; 2  lineBuffer[column] = frameBuffer[(lineInFrame << 4) | column]
.endm

  unroll initFromFrameBuffer     ; 64 0


  inc r16
  cp r16,100
  jne doneInterrupt
  mov r16, r2

doneInterrupt:
  sei                            ; 1  Enable interrupts a bit early so that we can postpone the epilogue until after the next interrupt if we need to.
  pop r0                         ; 2
  out 0x3f, r0                   ; 1
  pop r0                         ; 2
  pop r1                         ; 2
  pop r24                        ; 2
  pop r25                        ; 2
  pop r26                        ; 2
  pop r27                        ; 2
  pop r28                        ; 2
  pop r29                        ; 2
  pop r30                        ; 2
  pop r31                        ; 2
  reti                           ; 4


; The toolchain links in code that at the start of the program, sets SP = 0x8ff (top of RAM) and the status flags all to 0 before calling main.

.global main
main:

  ; Initialize hardware ports

  ; DDRB value:   0x2e  (Port B Data Direction Register)
  ;   DDB0           0  Sync in                     - input
  ;   DDB1           2  Blue LED (OC1A)             - output
  ;   DDB2           4  Shift register reset        - output
  ;   DDB3           8  Shift register data (MOSI)  - output
  ;   DDB4           0  Escape switch               - input
  ;   DDB5        0x20  Shift register clock (SCK)  - output
  ;   DDB6           0  XTAL1/TOSC1
  ;   DDB7           0  XTAL2/TOSC2
  ldi r31, 0x2e
  out 0x04, r31

  ; PORTB value:  0x11  (Port B Data Register)
  ;   PORTB0         1  Sync in                     - pull-up enabled
  ;   PORTB1         0  Blue LED (OC1A)
  ;   PORTB2         0  Shift register reset        - low
  ;   PORTB3         0  Shift register data (MOSI)
  ;   PORTB4      0x10  Escape switch               - pull-up enabled
  ;   PORTB5         0  Shift register clock (SCK)
  ;   PORTB6         0  XTAL1/TOSC1
  ;   PORTB7         0  XTAL2/TOSC2
  ldi r31, 0x11
  out 0x05, r31

  ; DDRC value:   0x07  (Port C Data Direction Register)
  ;   DDC0           1  Switch selector A           - output
  ;   DDC1           2  Switch selector B           - output
  ;   DDC2           4  Switch selector C           - output
  ;   DDC3           0  Decay potentiometer (ADC3)  - input
  ;   DDC4           0  Tempo potentiometer (ADC4)  - input
  ;   DDC5           0  Tuning potentiometer (ADC5) - input
  ;   DDC6           0  ~RESET
  ldi r31, 0x07
  out 0x07, r31

  ; PORTC value:  0x00  (Port C Data Register)
  ;   PORTC0         0  Switch selector A           - low
  ;   PORTC1         0  Switch selector B           - low
  ;   PORTC2         0  Switch selector C           - low
  ;   PORTC3         0  Volume potentiometer (ADC3)
  ;   PORTC4         0  Tempo potentiometer (ADC4)
  ;   PORTC5         0  Tuning potentiometer (ADC5)
  ;   PORTC6         0  ~RESET
  ldi r31, 0x00
  out 0x08, r31

  ; DDRD value:   0x6c  (Port D Data Direction Register)
  ;   DDD0           0  Debugging (RXD)
  ;   DDD1           0  Debugging (TXD)
  ;   DDD2           4  Sync out                    - output
  ;   DDD3           8  Audio output (OC2B)         - output
  ;   DDD4           0  Switch input 0              - input
  ;   DDD5        0x20  Red LED (OC0B)              - output
  ;   DDD6        0x40  Green LED (OC0A)            - output
  ;   DDD7           0  Switch input 1              - input
  ldi r31, 0x6c
  out 0x0a, r31

  ; PORTD value:  0x91  (Port D Data Register)
  ;   PORTD0         1  Debugging (RXD)             - pull-up enabled
  ;   PORTD1         0  Debugging (TXD)
  ;   PORTD2         0  Sync out                    - low
  ;   PORTD3         0  Audio output (OC2B)
  ;   PORTD4      0x10  Switch input 0              - pull-up enabled
  ;   PORTD5         0  Red LED (OC0B)
  ;   PORTD6         0  Green LED (OC0A)
  ;   PORTD7      0x80  Switch input 1              - pull-up enabled
  ldi r31, 0x91
  out 0x0b, r31

  ; TCCR0A value: 0xa3  (Timer/Counter 0 Control Register A)
  ;   WGM00          1  } Waveform Generation Mode = 3 (Fast PWM, TOP=0xff)
  ;   WGM01          2  }
  ;
  ;
  ;   COM0B0         0  } Compare Output Mode for Channel B: non-inverting mode
  ;   COM0B1      0x20  }
  ;   COM0A0         0  } Compare Output Mode for Channel A: non-inverting mode
  ;   COM0A1      0x80  }
  ldi r31, 0xa3
  out 0x24, r31

  ; TCCR0B value: 0x01  (Timer/Counter 0 Control Register B)
  ;   CS00           1  } Clock select: clkIO/1 (no prescaling)
  ;   CS01           0  }
  ;   CS02           0  }
  ;   WGM02          0  Waveform Generation Mode = 3 (Fast PWM, TOP=0xff)
  ;
  ;
  ;   FOC0B          0  Force Output Compare B
  ;   FOC0A          0  Force Output Compare A
  ldi r31, 0x01
  out 0x25, r31

  ; OCR0A value: green LED brightness
  ldi r31, 0x00
  out 0x27, r31
  ; OCR0B value: red LED brightness
  out 0x28, r31

  ; SPCR value:   0x50  (SPI Control Register)
  ;   SPR0           0  } SPI Clock Rate Select: fOSC/2
  ;   SPR1           0  }
  ;   CPHA           0  Clock Phase: sample on leading edge, setup on trailing edge
  ;   CPOL           0  Clock Polarity: leading edge = rising, trailing edge = falling
  ;   MSTR        0x10  Master/Slave Select: master
  ;   DORD           0  Data Order: MSB first
  ;   SPE         0x40  SPI Enable: enabled
  ;   SPIE           0  SPI Interrupt Enable: disabled
  ldi r31, 0x50
  out 0x2c, r31

  ; SPSR value:   0x01  (SPI Status Register)
  ;  SPI2X           1  SPI Clock Rate Select: fOSC/2
  ldi r31, 0x01
  out 0x2d, r31

  ; Set up stack:
  ldi r31, lo8(stackEnd-1)
  out 0x3d, r31
  ldi r31, hi8(stackEnd-1)
  out 0x3e, r31

  ; SPDR (SPI data register) port 0x2e - shift register data

  ; TIMSK0 value: 0x00  (Timer/Counter 0 Interrupt Mask Register)
  ;   TOIE0          0  Timer 0 overflow:  no interrupt
  ;   OCIE0A         0  Timer 0 compare A: no interrupt
  ;   OCIE0B         0  Timer 0 compare B: no interrupt
  ldi r31, 0x00
  sts 0x6e, r31

  ; TIMSK1 value: 0x01  (Timer/Counter 1 Interrupt Mask Register)
  ;   TOIE1          1  Timer 1 overflow:  interrupt
  ;   OCIE1A         0  Timer 1 compare A: no interrupt
  ;   OCIE1B         0  Timer 1 compare B: no interrupt
  ;
  ;
  ;   ICIE1          0  Timer 1 input capture: no interrupt
  ldi r31, 0x01
  sts 0x6f, r31

  ; TIMSK2 value: 0x00  (Timer/Counter 2 Interrupt Mask Register)
  ;   TOIE2          0  Timer 2 overflow:  no interrupt
  ;   OCIE2A         0  Timer 2 compare A: no interrupt
  ;   OCIE2B         0  Timer 2 compare B: no interrupt
  ldi r31, 0x00
  sts 0x70, r31

  ; ADCSRA value: 0xc7  (ADC Control and Status Register A)
  ;   ADPS0          1  ADC Prescaler Bit 0  }
  ;   ADPS1          2  ADC Prescaler Bit 1  } 16MHz/128 = 125KHz (between 50KHz and 200KHz)
  ;   ADPS2          4  ADC Prescaler Bit 2  }
  ;   ADIE           0  ADC Interrupt Enable: no interrupt
  ;   ADIF           0  ADC Interrupt Flag: no ack
  ;   ADATE          0  ADC Auto Trigger Enable: disabled
  ;   ADSC        0x40  ADC Start Conversion: not started
  ;   ADEN        0x80  ADC Enable: enabled
  ; ldi r31, 0xc7      ; Do this after programming ADMUX so the ADC knows which channel to read
  ; sts 0x7a, r31

  ; ADCSRB value: 0x00  (ADC Control and Status Register B)
  ;   ADTS0          0  ADC Auto Trigger Source Bit 0
  ;   ADTS1          0  ADC Auto Trigger Source Bit 1
  ;   ADTS2          0  ADC Auto Trigger Source Bit 2
  ;
  ;
  ;
  ;   ACME           0  Analog Comparator Multiplexer Enable: disabled
  ;
  ldi r31, 0x00
  sts 0x7b, r31

  ; ADMUX value:  0x44  (ADC Multiplexer Selection Register)
  ;   MUX0           0  Analog Channel Selection Bit 0  }
  ;   MUX1           0  Analog Channel Selection Bit 1  } Source is ADC4
  ;   MUX2           4  Analog Channel Selection Bit 2  }
  ;   MUX3           0  Analog Channel Selection Bit 3  }
  ;
  ;   ADLAR          0  ADC Left Adjust Result: not shifted
  ;   REFS0       0x40  Reference Selection Bit 0  } Voltage reference is AVCC
  ;   REFS1          0  Reference Selection Bit 1  }
  ldi r31, 0x44
  sts 0x7c, r31
  ldi r31, 0xc7   ; postponed from above
  sts 0x7a, r31

  ; DIDR0 value:  0x30  (Digital Input Disable Register 0)
  ;   ADC0D          0  ADC0 Digital Input Disable
  ;   ADC1D          0  ADC1 Digital Input Disable
  ;   ADC2D          0  ADC2 Digital Input Disable
  ;   ADC3D          0  ADC3 Digital Input Disable
  ;   ADC4D       0x10  ADC4 Digital Input Disable
  ;   ADC5D       0x20  ADC5 Digital Input Disable
  ldi r31, 0x30
  sts 0x7e, r31

  ; TCCR1A value: 0x82  (Timer/Counter 1 Control Register A)
  ;   WGM10          0  } Waveform Generation Mode = 14 (Fast PWM, TOP=ICR1)
  ;   WGM11          2  }
  ;
  ;
  ;   COM1B0         0  } Compare Output Mode for Channel B: normal port operation, OC1B disconnected
  ;   COM1B1         0  }
  ;   COM1A0         0  } Compare Output Mode for Channel A: non-inverting mode
  ;   COM1A1      0x80  }
  ldi r31, 0x82
  sts 0x80, r31

  ; TCCR1B value: 0x19  (Timer/Counter 1 Control Register B)
  ;   CS10           1  } Clock select: clkIO/1 (no prescaling)
  ;   CS11           0  }
  ;   CS12           0  }
  ;   WGM12          8  } Waveform Generation Mode = 14 (Fast PWM, TOP=ICR1)
  ;   WGM13       0x10  }
  ;
  ;   ICES1          0  Input Capture Edge Select: falling
  ;   ICNC1          0  Input Capture Noise Canceler: disabled
  ldi r31, 0x19
  sts 0x81, r31

  ; TCCR1C value: 0x00  (Timer/Counter 1 Control Register C)
  ;
  ;
  ;
  ;
  ;
  ;
  ;   FOC1B          0  Force Output Compare for Channel B
  ;   FOC1A          0  Force Output Compare for Channel A
  ldi r31, 0x00
  sts 0x82, r31

  ; ICR1 value: 0x0400  (Timer/Counter 1 Input Capture Register)
  ;   Timer 1 overflow frequency (15.625KHz at 16MHz clock frequency)
  ldi r31, 0x04
  sts 0x87, r31
  ldi r31, 0x00
  sts 0x86, r31

  ; OCR1A value: blue LED brightness
  ldi r31, 0x00
  sts 0x89, r31
  sts 0x88, r31

  ; TCCR2A value: 0x23  (Timer/Counter 2 Control Register A)
  ;   WGM20          1  } Waveform Generation Mode = 3 (Fast PWM, TOP=0xff)
  ;   WGM21          2  }
  ;
  ;
  ;   COM2B0         0  } Compare Output Mode for Channel B: non-inverting mode
  ;   COM2B1      0x20  }
  ;   COM2A0         0  } Compare Output Mode for Channel A: normal port operation, OC2A disconnected
  ;   COM2A1         0  }
  ldi r31, 0x23
  sts 0xb0, r31

  ; TCCR2B value: 0x01  (Timer/Counter 2 Control Register B)
  ;   CS20           1  } Clock select: clkIO/1 (no prescaling)
  ;   CS21           0  }
  ;   CS22           0  }
  ;   WGM22          0  Waveform Generation Mode = 3 (Fast PWM, TOP=0xff)
  ;
  ;
  ;   FOC2B          0  Force Output Compare B
  ;   FOC2A          0  Force Output Compare A
  ldi r31, 0x01
  sts 0xb1, r31

  ; OCR2B value: audio data
  ldi r31, 0x00
  sts 0xb4, r31

  ; UCSR0A value: 0x00  (USART Control and Status Register 0 A)
  ;   MPCM0          0  Multi-processor Communcation Mode: disabled
  ;   U2X0           0  Double the USART Transmission Speed: disabled
  ;
  ;
  ;
  ;
  ;   TXC0           0  USART Transmit Complete: not cleared
  ldi r31, 0x00
  sts 0xc0, r31

  ; UCSR0B value: 0xd8  (USART Control and Status Register 0 B)
  ;   TXB80          0  Transmit Data Bit 8 0
  ;
  ;   UCSZ02         0  Character Size 0: 8 bit
  ;   TXEN0          8  Transmitter Enable 0: enabled
  ;   RXEN0       0x10  Receiver Enable 0: enabled
  ;   UDRIE0         0  USART Data Register Empty Interrupt Enable 0: disabled
  ;   TXCIE0      0x40  TX Complete Interrupt Enable 0: enabled
  ;   RXCIE0      0x80  RX Complete Interrupt Enable 0: enabled
  ldi r31, 0xd8
  sts 0xc1, r31

  ; UCSR0C value: 0x06  (USART Control and Status Register 0 C)
  ;   UCPOL0         0  Clock Polarity
  ;   UCSZ00         2  Character Size: 8 bit
  ;   UCSZ01         4  Character Size: 8 bit
  ;   USBS0          0  Stop Bit Select: 1-bit
  ;   UPM00          0  Parity Mode: disabled
  ;   UPM01          0  Parity Mode: disabled
  ;   UMSEL00        0  USART Mode Select: asynchronous
  ;   UMSEL01        0  USART Mode Select: asynchronous
  ldi r31, 0x06
  sts 0xc2, r31

  ; UBRR0L value: 0x67  (USART Baud Rate Register Low) - 9600bps
  ldi r31, 0x67
  sts 0xc4, r31

  ; UBRR0H value: 0x00  (USART Baud Rate Register High)
  ldi r31, 0x00
  sts 0xc5, r31


; Initialize registers
  eor r2, r2                      ; r2 = 0
  mov r3, r2                      ; sampleInLine = 0
  mov r4, r2                      ; lineInFrame = 0
  mov r5, r2
  mov r6, r2                      ; frameInBeat = 0
  mov r7, r2
  ldi r31, 8
  mov r8, r31                     ; framesPerBeat = 0x800
  mov r9, r2                      ; beatInPattern = 0
  ldi r31, 6
  mov r10, r31                    ; lightPage = 6
  mov r11, r2                     ; switchInFrame = 0
  mov r12, r2                     ; (unused) = 0
  mov r13, r2                     ; (unused) = 0
  mov r14, r2                     ; beatsPerPattern = 0
  ldi r31, 4
  mov r15, r31                    ; waterPage = 4
  mov r16, r2                     ; lastSwitchPressed = 0
  ldi r31, 0x40
  mov r17, r31                    ; flags = 0  (waitingForSync = false,  lifeMode = false,  receivedSync = false,  outputSyncPulseActive = false,  gotRandom = false,  randomMode = false,  patternMode = true,  switchTouched = false)
  ldi r18, 1                      ; r18 = 1
  ldi r19, 0x10                   ; r19 = 0x10

; Initialize variables
  sts patternsPerLoop, r18
  ldi r31, 4
  sts flags2, r31  ; spaceAvailable = true


; Clear RAM. libgcc's __do_clear_bss will do this but then the entry code will
; call main, leaving the return address on the stack (in memory we want to
; use).
  ldi r31, 1
  ldi r30, 0
clearLoop:
  st Z+, r2
  cpi r31, 9
  brne clearLoop


; Copy some data from program memory to RAM. We do this here instead of
; putting it in .data and using libgcc's __do_copy_data because we might want
; to reference the program memory sine table again later (so that we can make
; the waveform user-modifyable, but still put it back to a sinewave without a
; reset).

  rcall copySine

  ldi r26, lo8(frequencies)        ; lo8(X)
  ldi r27, hi8(frequencies)        ; hi8(X)
  ldi r30, lo8(defaultFrequencies) ; lo8(Z)
  ldi r31, hi8(defaultFrequencies) ; hi8(Z)
copyDataLoop:                      ; do {
  lpm r0, Z+                       ;   r0 = flash[Z++]
  st X+, r0                        ;   ram[X++] = flash[Z++]
  cpi r26, lo8(frequencies + 0x20) ;
  brne copyDataLoop                ; } while (lo8(X) != frequencies + 0x20);

; Initialize microtoneKeys (the exact values don't matter - they just need to be all different)
  ldi r29, 0
  ldi r30, lo8(microtoneKeys)
  ldi r31, hi8(microtoneKeys)
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29
  inc r29
  st Z+, r29

  sei  ; enable interrupts

  jmp idleLoop


.section .bss

.global waveform
waveform:              ; 100-200
  .skip 0x100
.global frequencies
frequencies:           ; 200-220
  .skip 0x20
positions:             ; 220-240
  .skip 0x20
.global velocities
velocities:            ; 240-260
  .skip 0x20
.global volumes
volumes:               ; 260-270
  .skip 0x10
lineBuffer:            ; 270-280
  .skip 0x10
.global microtoneKeys
microtoneKeys:         ; 280-290
  .skip 0x10
randomData:            ; 290-294
  .skip 4
randomOn:              ; 294-295
  .skip 1
randomOff:             ; 295-296
  .skip 1
.global noiseUpdatePointer
noiseUpdatePointer:    ; 296-297
  .skip 1
patternInLoop:         ; 297-298
  .skip 1
.global patternsPerLoop
patternsPerLoop:       ; 298-299
  .skip 1
adcChannel:            ; 299-29a
  .skip 1
.global editor
editor:                ; 29a-29b
  .skip 1
.global waveformPreset
waveformPreset:        ; 29b-29c
  .skip 1
.global decayConstant
decayConstant:         ; 29c-29d
  .skip 1
.global decayConstantOverride
decayConstantOverride: ; 29d-29e
  .skip 1
.global framesPerBeatOverride
framesPerBeatOverride: ; 29e-2a0
  .skip 2
.global flags2
flags2:                ; 2a0-2a1  bit 0: fingerDown  bit 1: microtoneMode  bit 2: spaceAvailable  bit 3: fixedTuning  bit 4: updateNoise  bit 5: clearMode  bit 6: escapeTouched
  .skip 1
.global sendState
sendState:             ; 2a1-2a2
  .skip 1
.global receiveState
receiveState:          ; 2a2-2a3
  .skip 1
.global debugValue
debugValue:            ; 2a3-2a4
  .skip 1
.global debugAddress
debugAddress:          ; 2a4-2a6
  .skip 2
.global serialLED
serialLED:             ; 2a6-2a7
  .skip 1
.global mode
mode:                  ; 2a7-2a8
  .skip 1
nextFramesPerBeatLow:  ; 2a8-2a9
  .skip 1
nextFramesPerBeatHigh: ; 2a9-2aa
  .skip 1
nextLightsLit:         ; 2aa-2ab
  .skip 1
stack:                 ; 2ab-300  ; 85 bytes
  .skip 0x55
stackEnd:
.global frameBuffer
frameBuffer:           ; 300-400
  .skip 0x100
.global waterBuffers
waterBuffers:          ; 400-600
  .skip 0x200
.global lightBuffers
lightBuffers:          ; 600-800
  .skip 0x200
.global switchBuffer
switchBuffer:          ; 800-900
  .skip 0x100


; Register allocation:
;   multiplication result low   r0
;   multiplication result high  r1
;   0                           r2
;   sampleInLine                r3  (only 4 bits used - duplicates low bits of r11)
;   lineInFrame                 r4  (only 4 bits used - duplicates high bits of r11)
;   frameInBeat low             r5
;   frameInBeat high            r6
;   framesPerBeat low           r7
;   framesPerBeat high          r8
;   beatInPattern               r9  (only 4 bits used)
;   lightPage                   r10 (only 1 bit significant)
;   switchInFrame               r11
;   lastSwitch                  r12
;   switchesTouched             r13
;   beatsPerPattern             r14
;   waterPage                   r15 (only 1 bit significant)
;   random                      r16
;   flags                       r17 bit 0: waitingForSync  bit 1: lifeMode  bit 2: receivedSync  bit 3: outputSyncPulseActive  bit 4: gotRandom  bit 5: randomMode  bit 6: patternMode  bit 7: switchTouched
;   1                           r18
;   0x10                        r19
;   (scratch)                   r20
;   (scratch)                   r21
;   (scratch)                   r22
;   (scratch)                   r23
;   (scratch)                   r24
;   (scratch)                   r25
;   X low                       r26
;   X high                      r27
;   Y low                       r28
;   Y high                      r29
;   Z low                       r30
;   Z high                      r31


.section .progmem.data,"a",@progbits

.align 8


; table of words to send to the row-addressing shift registers. Rows are strobed one at a time in ascending order.
rowRegisterData:      ; 0f20 - 0f40
  .byte 0x00, 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x08
  .byte 0x00, 0x10, 0x00, 0x20, 0x00, 0x40, 0x00, 0x80
  .byte 0x01, 0x00, 0x02, 0x00, 0x04, 0x00, 0x08, 0x00
  .byte 0x10, 0x00, 0x20, 0x00, 0x40, 0x00, 0x80, 0x00


columnTable:          ; 0ff0 - 1000
  .word gs(column0), gs(column1), gs(column2), gs(column3), gs(column4), gs(column5), gs(column6), gs(column7)



.global font
font:          1248
  .byte 0x04 ; ..*..
  .byte 0x0a ; .*.*.
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x1f ; *****
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*

  .byte 0x0f ; ****.
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0f ; ****.
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0f ; ****.

  .byte 0x0e ; .***.
  .byte 0x11 ; *...*
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x11 ; *...*
  .byte 0x0e ; .***.

  .byte 0x0f ; ****.
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0f ; ****.

  .byte 0x1f ; *****
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x0f ; ****.
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x1f ; *****

  .byte 0x1f ; *****
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x0f ; ****.
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x01 ; *....

  .byte 0x0e ; .***.
  .byte 0x11 ; *...*
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x19 ; *..**
  .byte 0x11 ; *...*
  .byte 0x0e ; .***.

  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x1f ; *****
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*

  .byte 0x0e ; .***.
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x0e ; .***.

  .byte 0x10 ; ....*
  .byte 0x10 ; ....*
  .byte 0x10 ; ....*
  .byte 0x10 ; ....*
  .byte 0x10 ; ....*
  .byte 0x11 ; *...*
  .byte 0x0e ; .***.

  .byte 0x11 ; *...*
  .byte 0x09 ; *..*.
  .byte 0x05 ; *.*..
  .byte 0x03 ; **...
  .byte 0x05 ; *.*..
  .byte 0x09 ; *..*.
  .byte 0x11 ; *...*

  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x1f ; *****

  .byte 0x11 ; *...*
  .byte 0x1b ; **.**
  .byte 0x15 ; *.*.*
  .byte 0x15 ; *.*.*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*

  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x13 ; **..*
  .byte 0x15 ; *.*.*
  .byte 0x19 ; *..**
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*

  .byte 0x0e ; .***.
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0e ; .***.

  .byte 0x0f ; ****.
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0f ; ****.
  .byte 0x01 ; *....
  .byte 0x01 ; *....
  .byte 0x01 ; *....

  .byte 0x0e ; .***.
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x15 ; *.*.*
  .byte 0x09 ; *..*.
  .byte 0x16 ; .**.*

  .byte 0x0f ; ****.
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0f ; ****.
  .byte 0x05 ; *.*..
  .byte 0x09 ; *..*.
  .byte 0x11 ; *...*

  .byte 0x0e ; .***.
  .byte 0x11 ; *...*
  .byte 0x01 ; *....
  .byte 0x0e ; .***.
  .byte 0x10 ; ....*
  .byte 0x11 ; *...*
  .byte 0x0e ; .***.

  .byte 0x1f ; *****
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..

  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0e ; .***.

  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0a ; .*.*.
  .byte 0x0a ; .*.*.
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..

  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x15 ; *.*.*
  .byte 0x15 ; *.*.*
  .byte 0x15 ; *.*.*
  .byte 0x0a ; .*.*.

  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0a ; .*.*.
  .byte 0x04 ; ..*..
  .byte 0x0a ; .*.*.
  .byte 0x11 ; *...*
  .byte 0x11 ; *...*

  .byte 0x11 ; *...*
  .byte 0x11 ; *...*
  .byte 0x0a ; .*.*.
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..
  .byte 0x04 ; ..*..

  .byte 0x1f ; *****
  .byte 0x10 ; ....*
  .byte 0x08 ; ...*.
  .byte 0x04 ; ..*..
  .byte 0x02 ; .*...
  .byte 0x01 ; *....
  .byte 0x1f ; *****

  char *words[100]=
    {"ADVISOR","ANOTHER","ANTIQUE","AUDIBLE","CENTRAL","CENTURY","CERTAIN",
     "CHANGES","CHAPTER","CHARGED","CLUSTER","COMBINE","COMPLEX","COMPUTE",
     "CONFUSE","COUNTRY","COURAGE","CRYSTAL","DECIMAL","DEFIANT","DESTINY",
     "DETAILS","DIPLOMA","DOLPHIN","DRAWING","ECHOING","ENGLISH","ENTROPY",
     "EPSILON","EXACTLY","EXPLAIN","FIGURED","FLIGHTS","FORMULA","FORTUNE",
     "FRIENDS","GRAVITY","HARMONY","HOLIDAY","HONESTY","IMPULSE","INSTEAD",
     "JOURNAL","JOURNEY","JUPITER","JUSTICE","KINGDOM","LEADING","LIBERTY",
     "MACHINE","MAGNETS","MEDIANS","METHODS","MIRACLE","MIXTURE","MUSICAL",
     "NETWORK","NUCLEAR","NUMBERS","OBELISK","OBJECTS","OBSCURE","OPTIMAL",
     "ORANGES","ORGANIC","OUTLINE","PICTURE","PREDICT","PROBLEM","PROJECT",
     "PROMISE","QUALITY","QUICKLY","RAINBOW","READING","REALITY","REBUILD",
     "REPLICA","RESPOND","ROMANCE","ROUTINE","SCHOLAR","SECTION","SPECIAL",
     "SQUARED","STRANGE","SUBLIME","SURFACE","TALKING","TRIUMPH","TURBINE",
     "VERTIGO","VOLTAGE","VOYAGER","WHISPER","WHISTLE","WILDEST","WIZARDS",
     "WONDERS","WORSHIP"};

