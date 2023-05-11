
#define b11 2
#define b12 3
#define b13 4

#define b21 5
#define b23 6

#define b31 7
#define b32 8
#define b33 15

int lastTop = 0;
int lastRight = 0;
int lastBottom = 0;
int lastLeft = 0;

int lastAll = 0;
int allTapsCount = 0;

void singleTap(){Serial.println("•\tTap!");};
void doubleTap(){Serial.println("••\tDouble Tap!");};
void tripleTap(){Serial.println("•••\tTriple Tap!");};

void swipeRight(){Serial.println("→\tSwipe Right!");};
void swipeLeft(){Serial.println("←\tSwipe Left!");};
void swipeUp(){Serial.println("↑\tSwipe Up!");};
void swipeDown(){Serial.println("↓\tSwipe Down!");};

void setup() {
    pinMode(b11, INPUT);
    pinMode(b12, INPUT);
    pinMode(b13, INPUT);
    pinMode(b21, INPUT);
    pinMode(b23, INPUT);
    pinMode(b31, INPUT);
    pinMode(b32, INPUT);
    pinMode(b33, INPUT);
}

void checkButtons() {
    bool top, right, bottom, left, all;
    top = digitalRead(b11) || digitalRead(b12) || digitalRead(b13);
    right = digitalRead(b31) || digitalRead(b32) || digitalRead(b33);
    bottom = digitalRead(b31) || digitalRead(b32) || digitalRead(b33);
    left = digitalRead(b11) || digitalRead(b21) || digitalRead(b31);
    all = top && right && bottom && left;

    if(top) lastTop = millis();
    if(right) lastRight = millis();
    if(bottom) lastBottom = millis();
    if(left) lastLeft = millis();
    if(all) lastAll = millis();
}

void recognizeGestures() {
    if(allTapsCount == 3) tripleTap();
    int current = millis();
    int dx = lastLeft - lastRight;
    int dy = lastBottom - lastTop;
    int dz = current - lastAll;
    if(dx > 50 && dx < 200) swipeRight();
    else if(-dx > 50 && -dx < 20) swipeLeft();
    else if(dy > 50 && dy < 200) swipeUp();
    else if(-dy > 50 && -dy < 200) swipeDown();
    else if(dz > 50 && dz < 200){
        allTapsCount++;
        return;
    }
    else if(allTapsCount == 2) doubleTap();
    else if(allTapsCount == 1) singleTap();
    allTapsCount = 0;
}

void loop() {
    checkButtons();
    recognizeGestures();
}
