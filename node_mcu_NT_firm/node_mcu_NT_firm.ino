//#include <SocketIoClient.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <time.h>
#define ENABLE_READ_VCC
/**/
ESP8266WiFiMulti wifiMulti;
/* Voltaj Modu Ayarlanıyor Cihazdaki Voltajları Ölçmek İçin */
ADC_MODE(ADC_VCC);
/* Voltaj Durumlarını Ters Çevirmek İçin */
const int KAPALI = 1;
const int ACIK = 0;

/* Pinler LolIne Göre Yeniden Adresleniyor */

/* Saat Tarih Takibi İçin */
const int timezone = 3;
unsigned long tarihOncekiZaman = 0;        // Önceki Zaman Kaydediliyor Farkı Bulmak İçin
String zaman = "";

/* Socketio Tanıtılıyor */
//SocketIoClient webSocket;

/* Socketio Olay Mesaj Geldiğinde*/
void event(const char * payload, size_t length) {
  Serial.printf("got message: %s\n", payload);
}


/* Wifi İçin Giriş Şifreleri */
DNSServer dnsServer;
ESP8266WebServer server(80);

int ledModu = 0;

void ledModuDegistir(void)
{
  if (ledModu == 1) ledModu = 0; else ledModu = 1;
  server.sendHeader("Location", String("/"), true);
  server.send ( 303, "text/plain", "");
  delay(100);
}

/* Led Yakma Çağrısı İstemci Yak Sayfasını Çağırdığında Led Yakılıp Tekrar Ana Sayfaya Yönlendiriliyor */
void yak(void)
{
  digitalWrite(D4, ACIK);// Led Açık Konumuna Getiriliyor
  server.sendHeader("Location", String("/"), true);
  server.send ( 303, "text/plain", "");// Ana Sayfaya Yönlendiriliyor
  delay(100);
}

/* Led Söndürme Çağrısı İstemci Sondür Sayfasını Çağırdığında Led Söndürülüp Tekrar Ana Sayfaya Yönlendiriliyor */
void sondur(void)
{
  digitalWrite(D4, KAPALI);// Led Kapalı Duruma Getiriliyor
  server.sendHeader("Location", String("/"), true);
  server.send ( 303, "text/plain", "");// Ana Sayfaya Yönlendiriliyor
  delay(100);
}

void yenile(void)
{
  String veri = "<html><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/> <meta http-equiv=\"cache-control\" content=\"max-age=0\" /><meta http-equiv=\"cache-control\" content=\"no-cache\" /><meta http-equiv=\"expires\" content=\"0\" /><meta http-equiv=\"expires\" content=\"Tue, 01 Jan 1980 1:00:00 GMT\" /><meta http-equiv=\"pragma\" content=\"no-cache\" /><body><h1>FikoYT DijiSis</h1>";
  veri += "<div>";
  veri += zaman;
  veri += "</div>";
  veri += "<p>";
  if (ledModu == 1) {
    veri += "<p>Led yanıp Sönüyor</p>";
  } else {
    int ledDurum = digitalRead(D4);
    veri += ledDurum == ACIK ? "Led Yanıyor</p> <br> <a href=\"/sondur\"> Söndür </a>" : "Led Sönük</p> <br> <a href=\"/yak\"> Yak </a>";
    //veri += " <br><a href=\"/\"> Sayfayı Yenile </a>";
  }
  //  veri += " <br>Kablosuz Sinyal Gücü : %";
  //  veri += dbm2percent(WiFi.RSSI());
  veri += "<br>";
  veri += (ledModu) ? "Led Modu -> Yan Sön" : "Led Modu -> Bekle";
  veri += "<br> <a href=\"/ledModuDegistir\"> Led Modunu Değiştir </a>";
  veri += "<br>";
  veri += ESP.getVcc() / 1000.000 + 0.26;
  veri += "<br>";
  veri += "<br>Aloo ";
  veri += analogRead(A0) ;
  veri += "<br>";
  /* Pin Durumları Okunuyor */
  int durumD0 = digitalRead(D0);
  int durumD1 = digitalRead(D1);
  int durumD2 = digitalRead(D2);
  int durumD3 = digitalRead(D3);
  int durumD4 = digitalRead(D4);
  int durumD5 = digitalRead(D5);
  int durumD6 = digitalRead(D6);
  int durumD7 = digitalRead(D7);
  int durumD8 = digitalRead(D8);
  int durumD9 = digitalRead(D9);
  int durumD10 = digitalRead(D10);
  /* Pin Durumları Yazılıyor */
  veri += !(durumD0) ? "D0 Basılı" : "D0 Basılı Değil"; veri += "<br>";
  veri += !(durumD1) ? "D1 Basılı" : "D1 Basılı Değil"; veri += "<br>";
  veri += !(durumD2) ? "D2 Basılı" : "D2 Basılı Değil"; veri += "<br>";
  veri += !(durumD3) ? "D3 Basılı" : "D3 Basılı Değil"; veri += "<br>";
  veri += !(durumD4) ? "D4 Basılı" : "D4 Basılı Değil"; veri += "<br>";
  veri += !(durumD5) ? "D5 Basılı" : "D5 Basılı Değil"; veri += "<br>";
  veri += !(durumD6) ? "D6 Basılı" : "D6 Basılı Değil"; veri += "<br>";
  veri += !(durumD7) ? "D7 Basılı" : "D7 Basılı Değil"; veri += "<br>";
  veri += !(durumD8) ? "D8 Basılı" : "D8 Basılı Değil"; veri += "<br>";
  veri += !(durumD9) ? "D9 RX Basılı" : "D9 RX Basılı Değil"; veri += "<br>";
  veri += !(durumD10) ? "D10 TX Basılı" : "D10 TX Basılı Değil"; veri += "<br>";

  veri += "</body></html>";

  server.send(200, "text/html", veri);
  delay(100);
}

void json(void) {
  /* Pin Durumları Okunuyor */
  int durumD0 = digitalRead(D0);
  int durumD1 = digitalRead(D1);
  int durumD2 = digitalRead(D2);
  int durumD3 = digitalRead(D3);
  int durumD4 = digitalRead(D4);
  int durumD5 = digitalRead(D5);
  int durumD6 = digitalRead(D6);
  int durumD7 = digitalRead(D7);
  /* Json Yazılıyor */
  String html = "{";
  html += "D1:";
  html += !durumD1 ? "true," : "false,";
  html += "D2:";
  html += !durumD2 ? "true," : "false,";
  html += "D3:";
  html += !durumD3 ? "true," : "false,";
  html += "D4:";
  html += !durumD4 ? "true," : "false,";
  html += "D5:";
  html += !durumD5 ? "true," : "false,";
  html += "D6:";
  html += !durumD6 ? "true," : "false,";
  html += "D7:";
  html += !durumD7 ? "true," : "false,";
  html += "analog:";
  html += (String)analogRead(A0) + ",";
  html += "ip:";
  html += "'" + (String)WiFi.localIP().toString() + "'";
  html += "}";
  /* İstemciye Gönderiliyor */
  server.send(200, "application/json", html);
  delay(100);
}

String jsonRtrn(void) {
  /* Pin Durumları Okunuyor */
  int durumD0 = digitalRead(D0);
  int durumD1 = digitalRead(D1);
  int durumD2 = digitalRead(D2);
  int durumD3 = digitalRead(D3);
  int durumD4 = digitalRead(D4);
  int durumD5 = digitalRead(D5);
  int durumD6 = digitalRead(D6);
  int durumD7 = digitalRead(D7);
  /* Json Yazılıyor */
  String html = "{";
  html += "D1:";
  html += !durumD1 ? "true," : "false,";
  html += "D2:";
  html += !durumD2 ? "true," : "false,";
  html += "D3:";
  html += !durumD3 ? "true," : "false,";
  html += "D4:";
  html += !durumD4 ? "true," : "false,";
  html += "D5:";
  html += !durumD5 ? "true," : "false,";
  html += "D6:";
  html += !durumD6 ? "true," : "false,";
  html += "D7:";
  html += !durumD7 ? "true," : "false,";
  html += "analog:";
  html += (String)analogRead(A0) + ",";
  html += "ip:";
  html += "'" + (String)WiFi.localIP().toString() + "'";
  html += "}";
  /* İstemciye Gönderiliyor */
  return html;
}

/**
    Radyo Frekansının Desibel Olarak Tanımlanan Verisinin Yüzdeye Çevirilmesi
    Makine Dilinde Ve NodeMcu'da Sinyal Gücü dbm Olarak Verilir.Bu Dbm Olarak Verilen
    Değeri İnsanların Anlayabileceği Yüzde Olarak Değere Çeviren Fonksiyon
*/
int dbm2percent( int x ) {
  if (x < -92)
    return 1;
  else if (x > -21)
    return 100;
  else
    return round((-0.0154 * x * x) - (0.3794 * x) + 98.182);
}

/**
   Wifi Yayınlarını Tarayarak Listeleyen Ve Bunu Yayınlanan Serverda Sunabilen Fonksiyon
   Wifi Modülü İle Alakalı Tüm Özellikleri Fonksiyon İçinde Anlatmaya Çalıştım
*/
void wifiBilgi(void) {
  // Sunucu Sayfası Başlıkları Html Dilinde Ayarlanıyor İstemciye Veri Değişkeni İletilecek
  String veri = "<html><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/><h1>FikoYT Wifi Ağları</h1>";

  // Wifi Yayınları Taranıyor Ve Sayısı n Değişkenine Atanıyor
  int n = WiFi.scanNetworks();
  // Taramanın Tamamlandığını Bildiriyoruz.
  veri += "<div>Tarama Tamamlandı</div>";
  // Eğer Hiç Wifi Yayını Bulunamamış İse
  if (n == 0) {
    veri += "<div style='color:red'>Kablosuz Ağ bulunamadı</div>";
  }
  else
  {
    // Eğer Wifi Yayınları Bulunmuş İse Sayısı İstemciye Yazdırılıyor
    veri += n;
    veri += " Adet Kablosuz Ağ bulundu";
    // Bulunan Ağları Listelemek İçin Döngüye Giriliyor
    for (int i = 0; i < n; ++i)
    {
      veri += i + 1; // Sıra No Yazdırılıyor dan Başlamaması İçin 1 Ekleniyor
      veri += ": ";
      veri += WiFi.SSID(i); // Bulunan Wifinin İsmi Yazdırılıyor
      veri += " (";
      veri += "<progress value='" + (String)dbm2percent(WiFi.RSSI(i)) + "' max='100'></progress>"; //dbm Olarak Alınan Sinyal Gücü Yüzdeye Çeviriliyor
      veri += ") MAC:";
      veri += WiFi.BSSIDstr(i); // Wifi Cihazının Mac Adresi Yazılıyor
      veri += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " Güvenli Değil" : " Güvenli"; // Wifinin Güvenlik Tipi Yazdırılıyor
      veri += "<br>";
      veri += "<br>";
      delay(10);
    }
  }
  server.send(200, "text/html", veri); // Bilgiler İstemciye Gönderiliyor
}

/**
   Cihaz Açılışında (Boot Olayında) Ayarlar yapılıyor
   Pin Ayarları Wifi Bağlantısı Sunucu Hazırlama vs.
*/
void setup(void) {
  /*
    Pin Davranışları Ayarlanıyor
    INPUT_PULLUP Yerleşik Direnci Aktif Ediyor Direkt KısaDevre İle Devreyi Tamamlayabiliyoruz
  */
  pinMode(D0, INPUT_PULLUP);// D0 Pini İnput Olarak Ayarlanıyor Sürekli Açık Bilmiyorum (analog input)
  pinMode(D1, INPUT_PULLUP);// D1 Pini İnput Olarak Ayarlanıyor
  pinMode(D2, INPUT_PULLUP);// D2 Pini İnput Olarak Ayarlanıyor
  pinMode(D3, INPUT_PULLUP);// D3 Pini İnput Olarak Ayarlanıyor
  pinMode(D4, INPUT_PULLUP);// D4 Pini İnput Olarak Ayarlanıyor Led Pini İle Aynı Adrese Denk Geliyor
  pinMode(D5, INPUT_PULLUP);// D5 Pini İnput Olarak Ayarlanıyor
  pinMode(D6, INPUT_PULLUP);// D6 Pini İnput Olarak Ayarlanıyor
  pinMode(D7, INPUT_PULLUP);// D7 Pini İnput Olarak Ayarlanıyor
  pinMode(D8, INPUT_PULLUP);// D8 Pini İnput Olarak Ayarlanıyor Sürekli Açık Sanırım Bool Olayları İçin
  pinMode(D9, INPUT_PULLUP);// D9 Pini İnput Olarak Ayarlanıyor Sürekli Açık
  pinMode(D10, INPUT_PULLUP);// D10 Pini İnput Olarak Ayarlanıyor Sürekli Açık
  /* D8 Pini Boot Olaylarında Kontrol Ediliyor Giriş Çıkış İçin Şimdilik Uygun Değil Gibi */
  pinMode(2, OUTPUT);// Yerleşik Led Pini(NodeMcu'da 2 Nolu Olarak Geçiyor) Voltaj Verilecek Şekilde Ayarlanıyor


  digitalWrite(D4, KAPALI);
  Serial.begin(115200);
  /**/
  WiFi.mode(WIFI_AP_STA);
  wifiMulti.addAP("FikoYT_Yazılım", "FikoYT1299");
  wifiMulti.addAP("NTMETAL2", "@Ntmetal@");

  //WiFi.mode(WIFI_AP_STA);
  WiFi.softAP("NT_FIKOBOT_OTOMASYON", "FikoYT1299");


  //WiFi.begin(ssid, password);
  Serial.println("");
  // Bağlantı Yokken Döngüye Gir
  if (wifiMulti.run() == WL_CONNECTED) {
    Serial.println("");
    Serial.println("WiFi Bağlandı");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }

  dnsServer.setTTL(300);
  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  dnsServer.start(53, "*", IPAddress(192, 168, 4, 1));
  /*
    Serial.println("");
    Serial.println(ssid);
    Serial.print(" Kablosuz Adresine Bağlandı");
    Serial.print("IP adresi: ");
    Serial.println(WiFi.localIP());
    Serial.print("AP IP adresi: ");
    Serial.println(WiFi.localIP());
  */

  server.on("/yak", yak);
  server.on("/sondur", sondur);
  server.on("/", yenile);
  server.on("/wifi", wifiBilgi);
  server.on("/ledModuDegistir", ledModuDegistir);
  server.on("/json", json);

  server.begin();

  /**/
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  Serial.println("\nZaman İçin Bekleniyor");
  while (!time(nullptr)) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("\nZaman Alındı");

  /* Sockeio Kurulumları */
  //webSocket.on("event", event);
  //webSocket.beginSSL("192.168.1.100", 91, "socket.io/?EIO=3&transport=websocket", "7af49b92aa04ba5fdcf885dfd76269dd4578ce6a" );
}

/* Led Modu İçin Zaman Takip Değişkenleri */
unsigned long ledOncekiZaman = 0;        // Önceki Zaman Kaydediliyor Farkı Bulmak İçin
const long ledTekrarSure = 500;          // Tekrar Edilecek Saniye (milliSaniye)
int ledDurum = KAPALI;                // Performans Açısından Böyle Uygun Gibi
bool dnsAyarYapildi = false;



void loop(void) {
  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("WiFi Hala Bağlanamadı!");
    delay(1000);
  }

  if (!dnsAyarYapildi) {
    if (wifiMulti.run() == WL_CONNECTED) {
      /*
        dnsServer.setTTL(300);
        dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
        dnsServer.start(80, "fikoyt.com", WiFi.localIP());
      */
      Serial.println(WiFi.localIP());
      dnsAyarYapildi = true;
    }
  }

  /**/
  unsigned long simdikiZaman = millis();// Şimdiki zaman Alınıyor
  dnsServer.processNextRequest();
  server.handleClient();
  //  int btndurum = digitalRead(D2);
  //  if (ledModu == 1) {
  //    int ledDurum = digitalRead(D4);
  //    if ( ledDurum == ACIK) digitalWrite(led, KAPALI); else digitalWrite(led, ACIK);
  //    //delay(1000);
  //  }

  /* Eğer Led Modu 1(YanSön) İse */
  if (ledModu == 1) {
    /* Şimdiki zaman İle Tekrarlanan Süre Kontrol Ediliyor */
    if (simdikiZaman - ledOncekiZaman >= ledTekrarSure) {
      // Eğer Önceki Zaman + Tekrar Süre Büyükse Şimdiki Zamandan
      ledOncekiZaman = simdikiZaman; // Önceki Zaman Şimdiki Zamana Eşitleniyor

      // LedDurum Tersine Çevriliyor:
      if (ledDurum == KAPALI) {
        ledDurum = ACIK;
      } else {
        ledDurum = KAPALI;
      }
      // Durum Yerleşik Lede Ve D4'e Yazılıyor
      digitalWrite(D4, ledDurum);
    }
  }


  /* Zaman Ve Saati Yaz */
  if (simdikiZaman - tarihOncekiZaman >= 1000) {
    // Eğer Önceki Zaman + Tekrar Süre Büyükse Şimdiki Zamandan
    tarihOncekiZaman = simdikiZaman; // Önceki Zaman Şimdiki Zamana Eşitleniyor

    time_t now = time(nullptr);
    zaman = ctime(&now);
  }

  //webSocket.loop();
}
