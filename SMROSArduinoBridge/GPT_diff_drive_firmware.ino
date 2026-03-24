// Minimal sketch for Nano-side protocol:
// Receive:  CMD <left_rad_s> <right_rad_s>\n
// Send:     ENC <left_ticks_total> <right_ticks_total>\n

volatile long left_ticks = 0;
volatile long right_ticks = 0;

float left_cmd = 0.0;
float right_cmd = 0.0;

unsigned long last_report_ms = 0;
unsigned long last_cmd_ms = 0;

const unsigned long report_period_ms = 33;   // ~30 Hz
const unsigned long cmd_timeout_ms = 500;

// TODO: replace with your actual pins
const int LEFT_PWM = 5;
const int LEFT_IN1 = 6;
const int LEFT_IN2 = 7;
const int RIGHT_PWM = 9;
const int RIGHT_IN1 = 10;
const int RIGHT_IN2 = 11;

// TODO: replace with your encoder pins and ISR setup
void leftEncoderISR() {
  left_ticks++;
}

void rightEncoderISR() {
  right_ticks++;
}

void setup() {
  Serial.begin(57600);

  pinMode(LEFT_PWM, OUTPUT);
  pinMode(LEFT_IN1, OUTPUT);
  pinMode(LEFT_IN2, OUTPUT);
  pinMode(RIGHT_PWM, OUTPUT);
  pinMode(RIGHT_IN1, OUTPUT);
  pinMode(RIGHT_IN2, OUTPUT);

  // TODO: attach your encoder interrupts here
  // attachInterrupt(digitalPinToInterrupt(pin), leftEncoderISR, CHANGE);
  // attachInterrupt(digitalPinToInterrupt(pin), rightEncoderISR, CHANGE);

  stopMotors();
}

void stopMotors() {
  analogWrite(LEFT_PWM, 0);
  analogWrite(RIGHT_PWM, 0);
}

void setMotor(int pwm_pin, int in1, int in2, float rad_s) {
  int pwm = (int)constrain(fabs(rad_s) * 40.0, 0.0, 255.0);  // crude scale, tune this
  if (rad_s > 0.0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (rad_s < 0.0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
  analogWrite(pwm_pin, pwm);
}

void applyCommands() {
  setMotor(LEFT_PWM, LEFT_IN1, LEFT_IN2, left_cmd);
  setMotor(RIGHT_PWM, RIGHT_IN1, RIGHT_IN2, right_cmd);
}

void loop() {
  if (Serial.available()) {
    String line = Serial.readStringUntil('\n');
    line.trim();

    if (line.startsWith("CMD ")) {
      int first_space = line.indexOf(' ');
      int second_space = line.indexOf(' ', first_space + 1);
      if (second_space > 0) {
        String left_str = line.substring(first_space + 1, second_space);
        String right_str = line.substring(second_space + 1);
        left_cmd = left_str.toFloat();
        right_cmd = right_str.toFloat();
        applyCommands();
        last_cmd_ms = millis();
      }
    }
  }

  if (millis() - last_cmd_ms > cmd_timeout_ms) {
    left_cmd = 0.0;
    right_cmd = 0.0;
    stopMotors();
  }

  if (millis() - last_report_ms >= report_period_ms) {
    noInterrupts();
    long l = left_ticks;
    long r = right_ticks;
    interrupts();

    Serial.print("ENC ");
    Serial.print(l);
    Serial.print(" ");
    Serial.println(r);

    last_report_ms = millis();
  }
}