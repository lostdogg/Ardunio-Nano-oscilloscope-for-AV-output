# Ardunio-Nano-oscilloscope-for-AV-output
Ardunio Nano oscilloscope for AV output
Given the complexity of integrating the Arduino Nano, the TL072 op-amp circuit, five control potentiometers, and the composite video output, having a detailed wiring map and a structured assembly procedure is crucial. This is especially important for verifying those critical connections and ensuring the grounds are shared properly, preventing the high voltage issues you encountered earlier.

Here is a full breakdown, from planning your layout to your first test.

🗺️ Full Wiring Map
This diagram acts as your master guide. Every wire and component placement should match this layout. Pay close attention to the pin numbering on both the Arduino Nano and the TL072, and double-check all component values.

Key Wiring Sections (Referenced in the Procedure):

Power & Ground: The foundation of the entire build.

TL072 Signal Conditioning: The sensitive part that scales Eurorack voltages for the Arduino.

Control Potentiometers: Wiring for all five user controls.

TVout Composite Video: The resistor network for generating the video signal.

🏗️ Assembly Procedure
Follow these steps logically. Test frequently to isolate problems early. If you are building this as a module, consider building it on a breadboard first to verify functionality before soldering it onto a protoboard or PCB.

Step 1: Component Gathering and Planning
Gather all components listed in the updated BOM (Arduino Nano, TL072, RCA jack, 3.5mm jack, five 10k pots, resistors, capacitors, and a 10-pin power header).

Plan your layout. Identify where each major component will sit on your circuit board. If using a custom panel, plan the placement of pots and jacks relative to your circuit board.

Install and solder the 10-pin Eurorack power header first. This anchors your power rails (+12V, -12V, and GND).

Step 2: Wiring Power and Shared Ground
This step is absolutely critical for the scope's operation and preventing component damage.

Filter Power Rails: Connect a 10uF electrolytic capacitor between the +12V rail and Ground, and another between the -12V rail and Ground. Ensure correct polarity.

Power the TL072: Connect Pin 8 (V 
+
 ) to the +12V rail and Pin 4 (V 
−
 ) to the -12V rail.

Power the Arduino Nano: Connect the VIN pin to the +12V rail. Connect the Arduino's GND pin directly to the Eurorack Ground rail.

Verify Shared Ground: This is the most crucial check. Ensure there is solid continuity between the Eurorack Power Header Ground, the TL072 circuit Ground points, and the Arduino GND pin. Use your multimeter to confirm 0V between all these points.

Step 3: Building and Testing the TL072 Signal Conditioning Circuit
This section details the circuit responsible for protecting your Arduino from high Eurorack voltages.

Wiring the Circuit:

Stage 1: Buffer: Wire the first half of the TL072 as a unity-gain buffer. Input from your 3.5mm Eurorack input jack (Tip connection) goes to Pin 3 (Non-inverting Input 1). Connect Pin 1 (Output 1) directly to Pin 2 (Inverting Input 1) with a jumper wire.

2.5V Reference Voltage Divider: Create the 2.5V center reference for the signal using two 10kΩ resistors. Connect them in series between the Arduino's 5V pin and the common Ground. The connection point between the two resistors provides the 2.5V reference voltage. Connect this reference point to Pin 5 (Non-inverting Input 2) of the TL072. (Refer to the main wiring map for a clear visual).

Stage 2: Scaling and Offsetting: This part reduces the 24V peak-to-peak Eurorack signal (-12V to +12V) into a 0V to 5V signal suitable for the Arduino.

Connect Pin 1 (Output of Stage 1) to Pin 6 (Inverting Input 2) through a 100kΩ resistor (R 
in
​
 ).

Connect a 20kΩ resistor (R 
f
​
 ) between Pin 7 (Output 2) and Pin 6. This sets the 0.2x gain.

CRITICAL: Stop and Perform a Live Voltage Test (Before connecting to Arduino)

Do not proceed until you confirm the following voltages:

Measure Pin 5. It must be approximately 2.5V.

Measure Pin 7 with no input signal connected to the 3.5mm jack. It should read approximately 3.0V (based on our scaling math).

If either reading is significantly different (e.g., stuck near 0V or close to the +12V rail), re-check the wiring and resistor values for the 2.5V reference divider and the Stage 2 feedback loop. Once confirmed safe, you can connect TL072 Pin 7 to Arduino A0.

Step 4: Installing and Wiring Control Potentiometers
Connect the "top" outer leg of all five 10kΩ pots to the Arduino's 5V pin.

Connect the "bottom" outer leg of all five pots to the common Ground.

Connect the center wiper of each pot to its corresponding Arduino analog pin:

V-Div (Vertical Scale): A1

T-Div (Time Base): A2

Trigger Level: A3

H-Pos (Horizontal Position): A4

Mode (Acquisition Mode): A5

Step 5: Connecting the TVout Composite Video Output
This step creates the simple resistor network needed to combine sync and video signals for standard composite video output.

Sync Signal (Pin D9): Connect Arduino Pin D9 to a 1kΩ resistor.

Video Signal (Pin D7): Connect Arduino Pin D7 to a 470Ω resistor.

Combine Signals: Connect the free ends of both resistors together. This junction is your combined composite video signal.

Wire to RCA Jack: Connect this combined signal junction to the Center Pin of your RCA jack.

Ground the Jack: Connect the Outer Sleeve of the RCA jack to the common Ground.

Step 6: Software Installation and First Boot
Connect the Arduino Nano to your computer via USB.

Open the Arduino IDE. Install the necessary library: Sketch > Include Library > Manage Libraries... Search for and install "TVout".

Copy and paste the provided oscilloscope code (from the previous turn) into a new sketch. Ensure you select the correct board (Arduino Nano) and port in the IDE. Upload the code.

Disconnect USB and connect the composite video cable from the RCA jack to a composite input on a monitor or TV.

Power up your Eurorack case. The TVout display should show the grid and the scope interface.

Connect a test signal (like a slow LFO or an oscillator) to the Eurorack input jack.

Adjust the potentiometers to see how each control affects the displayed waveform and verify functionality.

🛰️ Module OverviewThis module converts high-voltage Eurorack signals ($\pm 12V$) into a visual waveform displayed on any NTSC composite monitor. It features a high-impedance input buffer, signal scaling, and three distinct acquisition modes to handle everything from slow LFOs to complex FM-frequency audio.🎛️ Panel ControlsControlFunctionDescriptionV-DIV (A1)Vertical ScaleAdjusts the zoom level of the waveform. Increasing this "stretches" the signal relative to the center zero-line.T-DIV (A2)Time BaseControls the sampling rate. Turn clockwise to "zoom in" on a single cycle of a high-frequency wave.TRIG (A3)Trigger LevelSets the voltage threshold at which the scope begins drawing. Use this to stabilize moving waveforms.H-POS (A4)Horizontal PanIntroduces a delay after the trigger occurs. This allows you to "look ahead" or scroll through a complex signal sequence.MODE (A5)AcquisitionA 3-way selector (via potentiometer) for Sample, Peak Detect, or Averaging modes.📊 The Display InterfaceThe Grid (Reticle)The display features a dashed-line crosshair.Horizontal Axis: Represents Time. The dots are spaced every 12 pixels.Vertical Axis: Represents Voltage. The dots are spaced every 16 pixels.Distortion Correction: The grid is mathematically "compressed" so that when viewed on a standard 16:9 widescreen TV, the horizontal stretch results in perfectly square proportions.Acquisition ModesSMPL (Sample): Standard operation. Displays the raw signal exactly as captured. Best for general-purpose monitoring.PEAK (Peak Detect): Captures the maximum and minimum excursions between samples. This will display a "thick" waveform, making it ideal for seeing high-frequency transients or noise that standard sampling might miss.AVG (Average): Smooths the signal by averaging four consecutive samples. Use this to clean up noisy signals or to see the fundamental frequency of a messy patch.⚡ Triggering LogicAuto vs. Normal ModeThe trigger behavior is determined by the D2 input state:Auto Mode (Pin D2 HIGH/Open): The scope will automatically refresh even if the signal does not cross the trigger threshold. This is useful for finding a "lost" signal or monitoring very slow LFOs.Normal Mode (Pin D2 LOW/Grounded): The display will only refresh if the signal crosses the TRIG threshold. The display will freeze on the last valid frame until a new trigger condition is met. This is essential for capturing non-repeating "one-shot" events.🛠️ Technical SpecificationsInput Impedance: $100k\Omega$ (Buffered via TL072).Input Voltage Range: $\pm 12V$ (Internal scaling to $0-5V$).Maximum Sample Rate: Approximately $15-20kHz$ (ADC clock limited).Display Resolution: $120 \times 96$ pixels.Output: Standard NTSC Composite Video ($1V_{pp}$).⚠️ Safety & CalibrationGrounding: Ensure your video monitor and Eurorack case share a common ground via the module's power header to prevent display flickering or "hum" bars.Vertical Center: If the waveform is not centered on the crosshair when no signal is present, verify your voltage divider is outputting exactly $2.5V$ to the op-amp reference pin.
