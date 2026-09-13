# Simple Clicker Yardım Rehberi

- Bu uygulamayı otomatik fare tıklama işleriniz için kullanabilirsiniz.
- UYARI: Bu uygulama kesinlikle oyunlarda oto-tıklamalı hileler için kullanılmamalıdır. Eğer kullanılırsa, bu uygulamanın geliştiricileri olarak biz bundan KESİNLİKLE sorumlu DEĞİLİZ. Garanti için lisansı okuyabilirsiniz.

## Ön Ayarlar

- İstediğiniz kadar ön ayarınız olabilir.
- Her ön ayarın kendi bir çeşit "ayar paketi" vardır.
- Biri bir çeşit iş için veya bir diğerini başka bir iş için kullanabilirsiniz.
- Sol menüden ön ayar ismine çift tıklayarak ona geçebilirsiniz.

### Ön Ayar Oluşturma
- Pencerenin sol üstünde bulunan `Ön Ayarlar` başlığının sağındaki `+` tuşuna basarak yeni bir ön ayar oluşturabilirsiniz.
- Oluşturduğunda, hemen bir isim yazarak ona istediğiniz bir ismi verebilirsiniz.
- Varsayılan olarak her oluşturulan ön ayar için `Yeni Ön Ayar N` şeklinde bir isim oluşturulur.
- Yeniden adlandırma bittiğinde, `ENTER`'a ya da ön ayar isminin sağındaki kaydet butonuna basarak yeni ismi kaydedebilirsiniz.
- Ya da isimlendirmeyi iptal etmek için `ESC`'ye veya kaydet butonunun sağındaki iptal butonuna basabilirsiniz.

### Ön Ayar Kaydet/İptal Etme
- Sağdan bir ayarı değiştirince ön ayarın ismi eğik ve kalın olmalı aynı zamanda bir yıldız ismin solunda belirmeli. Bu o ayarın değiştiğini gösterir.
- `Ctrl+S` veya kaydet butonuna basarak değişiklikleri kaydedebilirsiniz.
- `Ctrl+K` veya iptal butonuna basarak değişiklikleri iptal edebilirsiniz. (Değişiklikleri geri alır)

### Ön Ayar Yönetimi
- Bir ön ayarı silmek için önce fareyi o özel ön ayarın butonuna getirdikten sonra bir çöp kutusu butonu çıkmalı. Ona basarak ön ayarı silebilirsiniz.
- Bir ön ayarı yeniden adlandırmak için yine fareyi o özel ön ayara getirdikten sonra bir kalem butonu çıkmalı. Ona basınca bir yazı girdi kutusu çıkmalı. Oraya istediğiniz yeni ismi yazdıktan sonra `ENTER`a veya kaydet butonuna basarak isimlendirmeyi kaydedebilirsiniz. Ya da `ESC` veya iptal butonuna basarak isilendirmeyi iptal edebilirsiniz.
- Yeniden adlandırma yaparken hala ön ayarın ayarlarını değiştirebilirsiniz. Ama yeniden adlandırmayı kaydettiğinizde ön ayarı da kaydeder. Ve aynı şey isimlendirmeyi iptal ettiğinizde de geçerli. (değişiklikleri iptal eder)w

## Ön Ayar Ayarları
- Ön ayarların sağında gördüğünüz arayüzden aşağıdaki ayarları yapabilirsiniz.
- `Tıklama Aralığı` tıklamalar arası ne kadar zaman olacağını belirler. Sıfırdan büyük olmalı.
- `Tıklama Ayarları` fare tıklamalar hakkında detayları belirler.
- `Fare Düğmesi` tıklanacak fare düğmesidir. Sol, Sağ ve Orta seçebilirsiniz.
- `Tekrar` tıklamanın ne kadar tekrar edeceğini belirler. Sıfırdan büyük olmalı. Sınır 1 milyondur.
- `Durdurana Kadar Tekrar Et` seçildiğinde tıklayıcı, kısayol tuşuna basıp kapatılıncaya dek devam eder.
- `Fare İşaretçisi Konumu` tıklamanın gerçekleştirileceği monitörünüzdeki kesin konumdur. Birden fazla monitörünüz varsa bu çalışmayabilir.
- `Mevcut`'u seçerseniz X ve Y değerlerinin bir önemi kalmaz çünkü işaretçiniz nerdeyse oraya tıklama yapar.
- Kullanıcı arayüzünden konum seçmek için `Seç`'e bastıktan sonra konum seçmeniz için ekran donacak ve kararacak.
- İşaretçinizi istediğiniz konuma götürdükten sonra sol tıka basarak konumu seçebilirsiniz ve `ESC`'ye basarak iptal edin.

## Ayarlar
- Pencerenin altındaki `Ayarlar` butonuna basarak ayarlar menüsünü açabilirsiniz.
- Tema, dil ve kısayol tuşunu değiştirebilirsiniz.
- Sadece 2 tane dil destekleniyor: İngilizce ve Türkçe.
- Ve 2 tema: Koyu ve Açık
- Kısayol tuşunu değiştirmek için o yazı kutusuna tıklayın ve klaveyinzden bir tuşa ya da tuş kombinasyona basın
- Sonra kaydete basarak ayarları kaydedebilir ve iptale basarak değişiklikleri iptal edebilirsiniz.

## Kısayol Tuşu
- Kısayol tuşu uygulama kapanmadıkça her yerden uygulama tarafından dinlenir.
- Tuşa basarak tıklayıcıyı kapatıp açabilirsiniz.
- Bu davranış Wayland ve Linux'ta değişiyor.
- Wayland genel tuş atamalarını sadece uygulama içinde kısıtlıyor ama bu durum KDE Plasma'da geçerli değil.
- KDE genel tuş atamalarına XCB yoluyla izin veriyor ve bu yüzden uygulama XWayland üzerinden çalışıyor.
- X11'de Windows'taki gibi genel kısayollar sorunsuz çalışıyor.

## Otomatik Güncelleme
- Yeni bir güncelleme geldiğinde uygulama başlangıcında haberdar edilirsiniz.
- Yukarda `Yeni güncelleme var` gibi bir bildirim gelecek ve sağında bulunan `Şimdi Yükselt`'e basarak uygulamayı güncelleyebilirsiniz.
- Bu güncelleme tamamen otomatiktir, sizden sadece Windows'ta yönetici izinleri ve Linux'ta root yetkisi istenir.
- NOT: Bu yetkiler uygulamayı indirmek için de gereklidir. Eğer tüm kurulum sürecini arkada yapılan işlemleri görmek isterseniz proje açık kaynaktır, proje kök dizininde bulunan `aux` klasöründe hem Windows ISS script'i hem de Linux install/uninstall script'leri var.
- Güncelleme bitince yeni uygulama otomatik olarak açılır.

# Hakkında

- [ilpeN](https://github.com/ilpenSE) tarafından yapıldı.
- Qt kullanılarak C++'ta geliştirildi.
- Bu uygulama, [Auto Clicker](https://github.com/ilpenSE/autoclicker)'ın devamı ve basitleştirilmiş halidir.
- Lisans: Sadece GPL-3.0 (GNU General Public License)
