#include "oled_manager.h"

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C

OLEDManager::OLEDManager()
: display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1) {}

bool OLEDManager::begin() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        return false;
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.display();
    return true;
}

void OLEDManager::clear() {
    display.clearDisplay();
    display.display();
}

void OLEDManager::showWelcome() {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 10);
    display.println("CatWAN");
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println("Sistema NFC");
    display.display();
}

void OLEDManager::showWaiting() {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Sistema listo");
    display.setCursor(0, 40);
    display.println("Acerque tarjeta");
    display.display();
}

void OLEDManager::showWaitingCard() {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Esperando tarjeta");
    display.setCursor(0, 40);
    display.println("para registrar...");
    display.display();
}

void OLEDManager::showTimeout() {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Timeout");
    display.setCursor(0, 40);
    display.println("Sin tarjeta");
    display.display();
}

void OLEDManager::showCardAdded(const String& uid) {
    display.clearDisplay();
    display.setCursor(0, 10);
    display.println("Tarjeta");
    display.println("agregada:");
    display.setCursor(0, 40);
    display.println(uid);
    display.display();
}

void OLEDManager::showJSON(const String& json) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Payload:");
    display.println(json);
    display.display();
}

void OLEDManager::showCommand(String cmd) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.println("COMANDO:");
    display.println(cmd);

    display.display();
}


void OLEDManager::showMessage(String msg) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    display.println("MENSAJE");
    display.println("----------");

    display.println(msg);

    for (int i = 0; i < msg.length(); i += 20){
        display.println(msg.substring(i, i + 20));
    }

    display.display();
}

void OLEDManager::showCardAdded(String uid, String name) {
    display.clearDisplay();
    display.println("Tarjeta OK");
    display.println(name);
    display.println(uid);
    display.display();
}

void OLEDManager::showWelcomeName(String name) {
    display.clearDisplay();

    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Bienvenido");

    display.setTextSize(2);
    display.setCursor(0, 16);
    display.println(name);

    display.display();
}



