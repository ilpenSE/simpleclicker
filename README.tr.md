# SimpleClicker - Basit Bir Oto Tıklama Yazılımı

- Qt6 ve C++ ile yapıldı.
- Uygulamanın sol menüsünde özelleştirilebilir ön ayarlar mevcut
- Sol menüdeki `+` butonuna basarak yeni bir ön ayar oluşturabilirsiniz
- Tüm ön ayarlar, uygulama ayarları ve kayıtları makinenizdeki `AppData` veya `~/.local/share` klasörlerinde tutulur
- Bu, [AutoClicker](https://github.com/ilpenSE/autoclicker) projesinin devamıdır
- Not: Şu anda uygulama beta sürümündedir ve kararsız olabilir.
- Şu anlık sadece x86-64 mimarisiyle Windows ve GNU/Linux destekleniyor
- Linux'ta Wayland kullanıyorsanız uygulama X11 (XWayland) ile çalışıyor ve KDE'de fare tıklamaları için bir kerelik soru soruyor
- Ayrıca KDE'de global kısayollar XCB üzerinden çalışıyor ama bu durum GNOME için geçerli değil!
- Karşılaştığınız hataları uygulamayı geliştirmek için fikirlerinizi dilerseniz GitHub issues üzerinden aktarabilirsiniz

## Windows için Kurulum Rehberi

1. Aşağıdaki SimpleClicker.exe'yi indirip çalıştırın
2. Sonrakine basın, lisansı kabul edin sonraki ve kurun
3. Masaüstü kısayolu otomatik olarak oluşturulacaktır

## GNU/Linux için Kurulum Rehberi (Arch/Debian/Fedora tabanlılar)

1. İndirdiğiniz tarball'u açın
2. install.sh isimli bash script'ini çalıştırın
3. Bu script root yetkileri (sudo) kullandığından root şifrenizi girin
4. Gerekli paketleri indirip masaüstü entry'sini, çalıştırılabilir dosyayı ve simgeyi kopyalar
5. Script hangi paketleri indireceğini söyler
6. Kurulum bitince komut satırından ya da masaüstünden Simple Clicker'ı çalıştırabilirsiniz
7. Masaüstü kısayolu yine otomatik olarak oluşturulacaktır
