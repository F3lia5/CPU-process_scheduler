#include <stdio.h>

#define MAX 20       /* en fazla surec sayisi */
#define MAXCPU 200   /* bir surecin girilebilecek en buyuk CPU suresi */
#define MAXG 5000    /* Gantt'a eklenebilecek en fazla blok sayisi */
#define BLOK_SATIR 6 /* Gantt'ta bir satirdaki en fazla blok sayisi */
#define RENKLI                                                                 \
  1 /* 1: renkli arayuz, 0: duz metin (ANSI desteklemeyen                      \
       terminaller icin) */

/* ---------- GLOBAL DEGISKENLER ----------
   Gantt semasi uc algoritma tarafindan ve yazdirma fonksiyonu tarafindan
   ortak kullanilir; hepsini parametreyle tasimak yerine global tutuyoruz.
   Her algoritmadan once sifirla() ile sifirlanmalidir. */
int ganttNo[MAXG];  /* blokta calisan surec no (bosta icin 0) */
int ganttBas[MAXG]; /* blogun baslangic zamani */
int ganttBit[MAXG]; /* blogun bitis zamani */
int ganttSayac;     /* simdiye kadar eklenen blok sayisi */
int baglamSayac;    /* baglam degisimi sayisi */
int sonSurec;       /* en son calisan surecin numarasi (hic yoksa 0) */

/* ---------- ARAYUZ YARDIMCI FONKSIYONLARI ----------
   Terminali ANSI kacis kodlariyla temizler ve renklendirir. Bunlar sadece
   printf ile yazilan ozel karakter dizileridir, ek kutuphane gerekmez. */

/* Ekrani temizler ve imleci sol ust kosesine goturur. */
void ekranTemizle(void) {
  if (RENKLI) {
    printf("\033[2J\033[H");
  } else {
    printf("\n\n");
  }
}

/* Yazi rengini fg, arka plan rengini bg yapar (ANSI renk kodlari). */
void renkAc(int fg, int bg) {
  if (RENKLI) {
    printf("\033[%d;%dm", fg, bg);
  }
}

/* Kalin ve fg renginde yazi baslatir. */
void yaziRengiAc(int fg) {
  if (RENKLI) {
    printf("\033[1;%dm", fg);
  }
}

/* Tum renk ve bicim ayarlarini kapatir. */
void renkKapat(void) {
  if (RENKLI) {
    printf("\033[0m");
  }
}

/* Surec numarasina gore renk secer (6 renk, sirayla tekrar eder). */
void surecRengiAc(int surecNo) {
  int bg = 41 + (surecNo - 1) % 6;
  int fg = (bg == 42 || bg == 43 || bg == 46) ? 30 : 97;
  renkAc(fg, bg);
}

/* Girdi satirinin kalanini okuyup atar (scanf'ten sonra kalan Enter'i
   temizler). */
void satirTemizle(void) {
  int c;
  while ((c = getchar()) != '\n' && c != EOF) {
  }
}

/* Kullanici Enter'a basana kadar bekler. */
void enterBekle(void) {
  printf("\nDevam etmek icin Enter'a basin...");
  fflush(stdout);
  satirTemizle();
}

/* Surec sayisini, her surecin gelis zamanini ve CPU suresini klavyeden
   okur. Dizileri doldurur, surec sayisini *n ile geri verir. */
void veriOku(int no[], int gelis[], int cpu[], int *n) {
  do {
    printf("Lutfen surec sayisini girin (1 - %d): ", MAX);
    scanf("%d", n);
    if (*n < 1 || *n > MAX) {
      printf("Gecersiz sayi! Lutfen 1 ile %d arasinda bir sayi girin.\n", MAX);
    }
  } while (*n < 1 || *n > MAX);

  for (int i = 0; i < *n; i++) {
    no[i] = i + 1;
    printf("\n%d. Surec icin:\n", no[i]);
    do {
      printf("  Gelis zamani (>= 0): ");
      scanf("%d", &gelis[i]);
    } while (gelis[i] < 0);
    do {
      printf("  CPU suresi (1 - %d): ", MAXCPU);
      scanf("%d", &cpu[i]);
    } while (cpu[i] < 1 || cpu[i] > MAXCPU);
  }
}

/* Round Robin icin zaman dilimini (quantum) okur ve return ile verir. */
int quantumOku(void) {
  int q;
  do {
    printf("\nRound Robin zaman dilimi (quantum, >= 1): ");
    scanf("%d", &q);
  } while (q < 1);
  satirTemizle();
  return q;
}

/* Gantt sayaclarini ve baglam degisimi sayacini sifirlar.
   Her algoritmadan once cagrilir. */
void sifirla(void) {
  ganttSayac = 0;
  baglamSayac = 0;
  sonSurec = 0;
}

/* Gantt'a bir blok ekler. surecNo = 0 ise bosta blokudur.
   Blok, kendinden onceki son SUREC blogundan farkli bir surece aitse
   baglam degisimi sayaci 1 artar (ilk surec ve bosta sayilmaz). */
void ganttEkle(int surecNo, int bas, int bit) {
  ganttNo[ganttSayac] = surecNo;
  ganttBas[ganttSayac] = bas;
  ganttBit[ganttSayac] = bit;
  ganttSayac++;

  if (surecNo != 0) {
    if (sonSurec != 0 && sonSurec != surecNo) {
      baglamSayac++;
    }
    sonSurec = surecNo;
  }
}

/* Gantt semasini [P1 0-5] ve [BOS 18-20] bicimlerinde yazdirir.
   Her satirda en fazla BLOK_SATIR blok bulunur. */
void ganttYazdir(void) {
  printf("\nGantt Semasi:\n\n");
  for (int i = 0; i < ganttSayac; i++) {
    if (ganttNo[i] == 0) {
      renkAc(97, 100);
      printf(" [BOS %d-%d] ", ganttBas[i], ganttBit[i]);
    } else {
      surecRengiAc(ganttNo[i]);
      printf(" [P%d %d-%d] ", ganttNo[i], ganttBas[i], ganttBit[i]);
    }
    renkKapat();
    if ((i + 1) % BLOK_SATIR == 0 || i == ganttSayac - 1) {
      printf("\n\n");
    } else {
      printf(" ");
    }
  }
}

/* Aktif (bitmemis) surecler arasindan en erken gelis zamanini bulur.
   aktif[i] != 0 ise i. surec bitmemis demektir. */
int enErkenGelis(int gelis[], int aktif[], int n) {
  int enErken = -1;
  for (int i = 0; i < n; i++) {
    if (aktif[i] != 0 && (enErken == -1 || gelis[i] < enErken)) {
      enErken = gelis[i];
    }
  }
  return enErken;
}

/* Surec INDEKSLERINI gelis zamanina gore siralar (sira[] dizisine).
   Asil diziler degismez, boylece paralel diziler tutarli kalir.
   Araya ekleme siralamasi kararlidir: gelis esitse numarasi kucuk olan
   (indeksi kucuk olan) onde kalir. */
void gelisSirala(int gelis[], int sira[], int n) {
  for (int i = 0; i < n; i++) {
    sira[i] = i;
  }
  for (int i = 1; i < n; i++) {
    int anahtar = sira[i];
    int j = i - 1;
    while (j >= 0 && gelis[sira[j]] > gelis[anahtar]) {
      sira[j + 1] = sira[j];
      j--;
    }
    sira[j + 1] = anahtar;
  }
}

/* FCFS: surecler geliste gore, kesintisiz calisir.
   Gantt'i doldurur, bitis[] dizisini doldurur. */
void fcfs(int no[], int gelis[], int cpu[], int bitis[], int n) {
  int sira[MAX];
  int saat = 0;

  gelisSirala(gelis, sira, n);
  for (int k = 0; k < n; k++) {
    int p = sira[k];
    if (saat < gelis[p]) {
      ganttEkle(0, saat, gelis[p]);
      saat = gelis[p];
    }
    ganttEkle(no[p], saat, saat + cpu[p]);
    saat += cpu[p];
    bitis[p] = saat;
  }
}

/* SJF (kesintisiz): CPU bosaldiginda gelmis ve bitmemis surecler arasindan
   CPU suresi en kisa olan secilir. Esitlikte gelis zamani erken olan,
   o da esitse numarasi kucuk olan secilir. */
void sjf(int no[], int gelis[], int cpu[], int bitis[], int n) {
  int bekliyor[MAX];
  int saat = 0;
  int kalan = n;

  for (int i = 0; i < n; i++) {
    bekliyor[i] = 1;
  }

  while (kalan > 0) {
    int secilen = -1;
    for (int i = 0; i < n; i++) {
      if (bekliyor[i] && gelis[i] <= saat) {
        if (secilen == -1 || cpu[i] < cpu[secilen] ||
            (cpu[i] == cpu[secilen] && gelis[i] < gelis[secilen])) {
          secilen = i;
        }
      }
    }

    if (secilen == -1) {
      /* Kimse gelmemis: CPU bosta, en erken gelise atla */
      int enErken = enErkenGelis(gelis, bekliyor, n);
      ganttEkle(0, saat, enErken);
      saat = enErken;
    } else {
      ganttEkle(no[secilen], saat, saat + cpu[secilen]);
      saat += cpu[secilen];
      bitis[secilen] = saat;
      bekliyor[secilen] = 0;
      kalan--;
    }
  }
}

/* Round Robin: surecler P1..Pn sirasiyla dolasilir. Sirasi gelen surec
   gelmis ve bitmemisse en fazla quantum kadar calisir, degilse atlanir.
   Bir turda hic surec calismadiysa CPU bostadir: saat en erken gelise
   atlar ve yeni tur P1'den baslar. */
void roundRobin(int no[], int gelis[], int cpu[], int bitis[], int n,
                int quantum) {
  int kalanSure[MAX];
  int saat = 0;
  int biten = 0;

  for (int i = 0; i < n; i++) {
    kalanSure[i] = cpu[i];
  }

  while (biten < n) {
    int calisti = 0;

    for (int i = 0; i < n; i++) {
      if (kalanSure[i] > 0 && gelis[i] <= saat) {
        int sure = kalanSure[i] < quantum ? kalanSure[i] : quantum;
        ganttEkle(no[i], saat, saat + sure);
        saat += sure;
        kalanSure[i] -= sure;
        if (kalanSure[i] == 0) {
          bitis[i] = saat;
          biten++;
        }
        calisti = 1;
      }
    }

    if (!calisti) {
      int enErken = enErkenGelis(gelis, kalanSure, n);
      ganttEkle(0, saat, enErken);
      saat = enErken;
    }
  }
}

/* Bitis zamanlarindan donus (bitis - gelis) ve bekleme (donus - cpu)
   sürelerini hesaplayip donus[] ve bekleme[] dizilerine yazar. */
void hesapla(int gelis[], int cpu[], int bitis[], int donus[], int bekleme[],
             int n) {
  for (int i = 0; i < n; i++) {
    donus[i] = bitis[i] - gelis[i];
    bekleme[i] = donus[i] - cpu[i];
  }
}

/* Sonuc tablosunu surec numarasi sirasiyla yazdirir. */
void tabloYazdir(int no[], int gelis[], int cpu[], int bitis[], int donus[],
                 int bekleme[], int n) {
  printf("\nSonuc Tablosu:\n");
  yaziRengiAc(36);
  printf("%-5s %-7s %-6s %-7s %-7s %-8s\n", "Surec", "Gelis", "CPU", "Bitis",
         "Donus", "Bekleme");
  renkKapat();
  for (int i = 0; i < n; i++) {
    surecRengiAc(no[i]);
    printf(" P%-2d ", no[i]);
    renkKapat();
    printf(" %-7d %-6d %-7d %-7d %-8d\n", gelis[i], cpu[i], bitis[i], donus[i],
           bekleme[i]);
  }
}

/* Ortalama donus ve ortalama bekleme suresini hesaplar.
   Iki sonuc dondurmek gerektigi icin return yerine pointer parametre
   kullanir: sonuclar *ortDonus ve *ortBekleme ile geri verilir. */
void ortalamalar(int donus[], int bekleme[], int n, double *ortDonus,
                 double *ortBekleme) {
  int toplamDonus = 0;
  int toplamBekleme = 0;

  for (int i = 0; i < n; i++) {
    toplamDonus += donus[i];
    toplamBekleme += bekleme[i];
  }
  *ortDonus = (double)toplamDonus / n;
  *ortBekleme = (double)toplamBekleme / n;
}

/* Bir algoritmanin tum sonucunu yazdirir: hesaplama, Gantt, tablo ve ozet.
   Ortalama bekleme suresini return ile geri verir (karsilastirma icin). */
double raporYazdir(int no[], int gelis[], int cpu[], int bitis[], int donus[],
                   int bekleme[], int n) {
  double ortDonus;
  double ortBekleme;

  hesapla(gelis, cpu, bitis, donus, bekleme, n);
  ganttYazdir();
  tabloYazdir(no, gelis, cpu, bitis, donus, bekleme, n);
  ortalamalar(donus, bekleme, n, &ortDonus, &ortBekleme);

  printf("\nOrtalama bekleme suresi: %.2f\n", ortBekleme);
  printf("Ortalama donus suresi  : %.2f\n", ortDonus);
  printf("Baglam degisimi sayisi : %d\n", baglamSayac);
  return ortBekleme;
}

/* Algoritma basligini yazdirir. static sayac, fonksiyon her cagrildiginda
   sifirlanmaz: scope sadece bu fonksiyon, omru ise programin sonuna kadar. */
void baslikYazdir(const char ad[]) {
  static int sayac = 0;
  sayac++;
  yaziRengiAc(36);
  printf("=========================================\n");
  printf(" %d. Algoritma: %s\n", sayac, ad);
  printf("=========================================\n");
  renkKapat();
}

/* Bir degeri, en buyuk degere oranli uzunlukta (en fazla 40 karakter) renkli
   cubuk olarak yazdirir. Renk kapaliysa cubuk '#' karakteriyle cizilir. */
void cubukYazdir(const char ad[], double deger, double enBuyuk, int bg) {
  int uzunluk = 0;

  if (enBuyuk > 0) {
    uzunluk = (int)(deger / enBuyuk * 40 + 0.5);
  }
  printf("%-12s ", ad);
  renkAc(97, bg);
  for (int i = 0; i < uzunluk; i++) {
    putchar(RENKLI ? ' ' : '#');
  }
  renkKapat();
  printf(" %.2f\n", deger);
}

/* Girilen surec bilgilerini ve zaman dilimini bir tablo halinde yazar. */
void girdiOzetiYazdir(int no[], int gelis[], int cpu[], int n, int quantum) {
  yaziRengiAc(36);
  printf("=========================================\n");
  printf(" GIRILEN SUREC BILGILERI\n");
  printf("=========================================\n");
  renkKapat();
  printf("\n%-5s %-7s %-6s\n", "Surec", "Gelis", "CPU");
  for (int i = 0; i < n; i++) {
    surecRengiAc(no[i]);
    printf(" P%-2d ", no[i]);
    renkKapat();
    printf(" %-7d %-6d\n", gelis[i], cpu[i]);
  }
  printf("\nRound Robin zaman dilimi (quantum): %d\n", quantum);
}

/* Uc algoritmanin ortalama bekleme surelerini yan yana yazar ve en dusugu
   belirtir. Esitlikte FCFS, SJF, Round Robin sirasiyla ilki secilir. */
void karsilastir(double ortFcfs, double ortSjf, double ortRr) {
  double enBuyuk = ortFcfs;
  if (ortSjf > enBuyuk) {
    enBuyuk = ortSjf;
  }
  if (ortRr > enBuyuk) {
    enBuyuk = ortRr;
  }

  yaziRengiAc(36);
  printf("=========================================\n");
  printf(" KARSILASTIRMA (ortalama bekleme suresi)\n");
  printf("=========================================\n");
  renkKapat();
  printf("\nFCFS: %.2f    SJF: %.2f    Round Robin: %.2f\n\n", ortFcfs, ortSjf,
         ortRr);

  cubukYazdir("FCFS", ortFcfs, enBuyuk, 44);
  cubukYazdir("SJF", ortSjf, enBuyuk, 42);
  cubukYazdir("Round Robin", ortRr, enBuyuk, 45);

  printf("\n");
  yaziRengiAc(32);
  if (ortFcfs <= ortSjf && ortFcfs <= ortRr) {
    printf("En dusuk ortalama bekleme: FCFS\n");
  } else if (ortSjf <= ortRr) {
    printf("En dusuk ortalama bekleme: SJF\n");
  } else {
    printf("En dusuk ortalama bekleme: Round Robin\n");
  }
  renkKapat();
}

int main() {
  int no[MAX], gelis[MAX], cpu[MAX];
  int bitis[MAX], donus[MAX], bekleme[MAX];
  int n, quantum;
  double ortFcfs, ortSjf, ortRr;

  ekranTemizle();
  yaziRengiAc(36);
  printf("=========================================\n");
  printf(" CPU SUREC ZAMANLAYICI SIMULATORU\n");
  printf("=========================================\n\n");
  renkKapat();
  veriOku(no, gelis, cpu, &n);
  quantum = quantumOku();

  ekranTemizle();
  girdiOzetiYazdir(no, gelis, cpu, n, quantum);
  enterBekle();

  ekranTemizle();
  sifirla();
  baslikYazdir("FCFS");
  fcfs(no, gelis, cpu, bitis, n);
  ortFcfs = raporYazdir(no, gelis, cpu, bitis, donus, bekleme, n);
  enterBekle();

  ekranTemizle();
  sifirla();
  baslikYazdir("SJF (kesintisiz)");
  sjf(no, gelis, cpu, bitis, n);
  ortSjf = raporYazdir(no, gelis, cpu, bitis, donus, bekleme, n);
  enterBekle();

  ekranTemizle();
  sifirla();
  baslikYazdir("Round Robin");
  printf("Zaman dilimi (quantum): %d\n", quantum);
  roundRobin(no, gelis, cpu, bitis, n, quantum);
  ortRr = raporYazdir(no, gelis, cpu, bitis, donus, bekleme, n);
  enterBekle();

  ekranTemizle();
  karsilastir(ortFcfs, ortSjf, ortRr);
  printf("\n");
  return 0;
}
