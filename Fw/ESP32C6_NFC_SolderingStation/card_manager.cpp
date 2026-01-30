#include "card_manager.h"

// Define the cards file path constant
#define CARDS_FILE "/cards.txt"
#define LINE_BUFFER_SIZE 64

CardManager::CardManager() : cardCount(0), initialized(false) {
    for (int i = 0; i < MAX_AUTHORIZED_CARDS; i++) {
        cards[i].uid[0] = '\0';
        cards[i].name[0] = '\0';
        cards[i].relayIndex = -1;
    }
}

bool CardManager::begin() {
    if (!SPIFFS.begin(true)) {
        #if DEBUG_SERIAL
        Serial.println("SPIFFS error");
        #endif
        initialized = false;
        return false;
    }

    #if DEBUG_SERIAL
    Serial.println("SPIFFS OK");
    #endif

    loadCardsFromSPIFFS();
    initialized = true;
    return true;
}

bool CardManager::loadCardsFromSPIFFS() {
    if (!SPIFFS.exists(CARDS_FILE)) return false;

    File file = SPIFFS.open(CARDS_FILE, "r");
    if (!file) return false;

    cardCount = 0;

   char line[LINE_BUFFER_SIZE];

    while (file.available() && cardCount < MAX_AUTHORIZED_CARDS) {

        int len = file.readBytesUntil('\n', line, LINE_BUFFER_SIZE - 1);
        if (len <= 0) continue;

        line[len] = '\0';

        // quitar \r
        char* cr = strchr(line, '\r');
        if (cr) *cr = '\0';

        if (strlen(line) == 0) continue;

        char* uid   = strtok(line, "|");
        char* name  = strtok(NULL, "|");
        char* relay = strtok(NULL, "|");

        if (!uid || !name) continue;

        strncpy(cards[cardCount].uid, uid, UID_MAX_LEN - 1);
        cards[cardCount].uid[UID_MAX_LEN - 1] = '\0';

        strncpy(cards[cardCount].name, name, NAME_MAX_LEN - 1);
        cards[cardCount].name[NAME_MAX_LEN - 1] = '\0';

        cards[cardCount].relayIndex = relay ? atoi(relay) : -1;

        cardCount++;
    }


    file.close();
    
    #if DEBUG_SERIAL
    Serial.print("Cargadas ");
    Serial.print(cardCount);
    Serial.println(" tarjetas");
    #endif
    
    return true;
}

bool CardManager::saveCardsToSPIFFS() {
    File file = SPIFFS.open(CARDS_FILE, "w");
    if (!file) return false;

    for (int i = 0; i < cardCount; i++) {
        file.print(cards[i].uid);
        file.print("|");
        file.print(cards[i].name);

        if (cards[i].relayIndex >= 0) {
            file.print("|");
            file.print(cards[i].relayIndex);
        }
        file.println();
    }

    file.close();
    return true;
}

int CardManager::findCardIndex(const char* uid) {
    if(!uid) return -1;

    for (int i = 0; i < cardCount; i++) {
        if (strcmp(cards[i].uid, uid) == 0) return i;
    }
    return -1;
}

bool CardManager::addCard(String uid, String name) {
    uid.toUpperCase();
    uid.trim();
    name.trim();

    if (!uid.length() || !name.length()) {
        #if DEBUG_SERIAL
        Serial.println("Error: UID o nombre vacío");
        #endif
        return false;
    }
    
    if (findCardIndex(uid.c_str()) >= 0) {
        #if DEBUG_SERIAL
        Serial.println("Error: Tarjeta ya existe");
        #endif
        return false;
    }
    
    if (cardCount >= MAX_AUTHORIZED_CARDS) {
        #if DEBUG_SERIAL
        Serial.println("Error: Límite de tarjetas alcanzado");
        #endif
        return false;
    }

    strncpy(cards[cardCount].uid, uid.c_str(), UID_MAX_LEN - 1);
    cards[cardCount].uid[UID_MAX_LEN - 1] = '\0';

    strncpy(cards[cardCount].name, name.c_str(), NAME_MAX_LEN - 1);
    cards[cardCount].name[NAME_MAX_LEN - 1] = '\0';

    cards[cardCount].relayIndex = -1;
    cardCount++;

    return saveCardsToSPIFFS();
}

bool CardManager::removeCard(String uid) {
    int index = findCardIndex(uid.c_str());
    if (index < 0) {
        #if DEBUG_SERIAL
        Serial.println("Error: Tarjeta no encontrada");
        #endif
        return false;
    }

    // Desplazar todas las tarjetas hacia arriba
    for (int i = index; i < cardCount - 1; i++) {
        cards[i] = cards[i + 1];
    }

    cardCount--;
    
    #if DEBUG_SERIAL
    Serial.println("Tarjeta removida exitosamente");
    #endif
    
    return saveCardsToSPIFFS();
}

bool CardManager::isCardAuthorized(String uid) {
    return findCardIndex(uid.c_str()) >= 0;
}

String CardManager::getCardName(String uid) {
    uid.toUpperCase();
    int index = findCardIndex(uid.c_str());
    if (index >= 0) return cards[index].name;
    return "";
}

String CardManager::getCardAt(int index) {
    if (index >= 0 && index < cardCount) {
        return cards[index].uid;
    }
    return "";
}

bool CardManager::assignCardToRelay(String uid, int relayIndex) {
    int index = findCardIndex(uid.c_str());
    if (index < 0) {
        #if DEBUG_SERIAL
        Serial.println("Error: Tarjeta no encontrada");
        #endif
        return false;
    }

    cards[index].relayIndex = relayIndex;
    
    #if DEBUG_SERIAL
    Serial.print("Tarjeta ");
    Serial.print(uid);
    Serial.print(" asignada a relé ");
    Serial.println(relayIndex);
    #endif
    
    return saveCardsToSPIFFS();
}

int CardManager::getCardRelay(String uid) {
    int index = findCardIndex(uid.c_str());
    if (index < 0) return -1;
    return cards[index].relayIndex;
}

void CardManager::listCards() {
    Serial.println("\n=== Tarjetas Autorizadas ===");
    
    if (cardCount == 0) {
        Serial.println("No hay tarjetas registradas");
        Serial.println("============================\n");
        return;
    }
    
    for (int i = 0; i < cardCount; i++) {
        Serial.print(i + 1);
        Serial.print(". UID: ");
        Serial.print(cards[i].uid);
        Serial.print(" | Nombre: ");
        Serial.print(cards[i].name);
        Serial.print(" | Relé: ");
        
        if (cards[i].relayIndex >= 0) {
            Serial.print(cards[i].relayIndex);
            Serial.print(" (Cautín ");
            Serial.print(cards[i].relayIndex + 1);
            Serial.println(")");
        } else {
            Serial.println("Sin asignar");
        }
    }
    
    Serial.print("\nTotal: ");
    Serial.print(cardCount);
    Serial.println(" tarjetas");
    Serial.println("============================\n");
}

void CardManager::listAssignments() {
    Serial.println("\n=== Asignaciones Tarjeta-Cautín ===");
    
    if (cardCount == 0) {
        Serial.println("No hay asignaciones");
        Serial.println("===================================\n");
        return;
    }
    
    int assignedCount = 0;
    
    for (int i = 0; i < cardCount; i++) {
        if (cards[i].relayIndex >= 0) {
            assignedCount++;
            Serial.print(assignedCount);
            Serial.print(". UID: ");
            Serial.print(cards[i].uid);
            Serial.print(" | Nombre: ");
            Serial.print(cards[i].name);
            Serial.print(" | Cautín: ");
            Serial.println(cards[i].relayIndex + 1);
        }
    }
    
    if (assignedCount == 0) {
        Serial.println("No hay tarjetas asignadas a cautines");
    } else {
        Serial.print("\nTotal asignadas: ");
        Serial.println(assignedCount);
    }
    
    Serial.println("===================================\n");
}

void CardManager::clearAllCards() {
    // Limpiar el array en memoria
    cardCount = 0;
    for (int i = 0; i < MAX_AUTHORIZED_CARDS; i++) {
        cards[i].uid[0] = '\0';
        cards[i].name[0] = '\0';
        cards[i].relayIndex = -1;
    }
    
    // Eliminar el archivo y recrearlo vacío
    if (SPIFFS.remove(CARDS_FILE)) {
        File file = SPIFFS.open(CARDS_FILE, "w");
        if (file) {
            file.close();
        }
        
        #if DEBUG_SERIAL
        Serial.println("Todas las tarjetas han sido eliminadas");
        #endif
    } else {
        #if DEBUG_SERIAL
        Serial.println("Error al eliminar tarjetas");
        #endif
    }
}