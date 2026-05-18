#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <cmath>

enum class GameState { MENU, DIFFICULTY_SELECT, PLAYING };

struct GameOptions {
    bool isHardMode = false;
    float timeLimit = 60.0f;
};

sf::ConvexShape buatKotakMelengkung(float width, float height, float radius, sf::Color color) {
    sf::ConvexShape shape;
    unsigned int quality = 8;
    shape.setPointCount(quality * 4);
    float angle = 0;
    for (unsigned int i = 0; i < quality; ++i) {
        angle = static_cast<float>(i) * (90.f / static_cast<float>(quality - 1)) * 3.14159265f / 180.f;
        shape.setPoint(i, sf::Vector2f(width - radius + std::cos(angle) * radius, radius - std::sin(angle) * radius));
    }
    for (unsigned int i = 0; i < quality; ++i) {
        angle = static_cast<float>(i) * (90.f / static_cast<float>(quality - 1)) * 3.14159265f / 180.f;
        shape.setPoint(quality + i, sf::Vector2f(radius - std::sin(angle) * radius, radius - std::cos(angle) * radius));
    }
    for (unsigned int i = 0; i < quality; ++i) {
        angle = static_cast<float>(i) * (90.f / static_cast<float>(quality - 1)) * 3.14159265f / 180.f;
        shape.setPoint(quality * 2 + i, sf::Vector2f(radius - std::cos(angle) * radius, height - radius + std::sin(angle) * radius));
    }
    for (unsigned int i = 0; i < quality; ++i) {
        angle = static_cast<float>(i) * (90.f / static_cast<float>(quality - 1)) * 3.14159265f / 180.f;
        shape.setPoint(quality * 3 + i, sf::Vector2f(width - radius + std::sin(angle) * radius, height - radius + std::cos(angle) * radius));
    }
    shape.setFillColor(color);
    return shape;
}

std::vector<sf::Color> hitungWarnaBaris(std::string tebakan, std::string jawaban) {
    std::vector<sf::Color> hasil(5, sf::Color(58, 58, 60));
    std::vector<bool> jawabanTerpakai(5, false);
    std::vector<bool> tebakanTerpakai(5, false);
    for (int i = 0; i < 5; i++) {
        if (tebakan[i] == jawaban[i]) {
            hasil[i] = sf::Color(83, 141, 78);
            jawabanTerpakai[i] = true; tebakanTerpakai[i] = true;
        }
    }
    for (int i = 0; i < 5; i++) {
        if (!tebakanTerpakai[i]) {
            for (int j = 0; j < 5; j++) {
                if (!jawabanTerpakai[j] && tebakan[i] == jawaban[j]) {
                    hasil[i] = sf::Color(181, 159, 59);
                    jawabanTerpakai[j] = true; break;
                }
            }
        }
    }
    return hasil;
}

int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(600, 900)), "Katla - Tebak Kata");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) return -1;
    }

    sf::Texture texLogo, texBoardFull;
    if (!texLogo.loadFromFile("C:/Katla/Katla/logo.jpeg") ||
        !texBoardFull.loadFromFile("C:/Katla/Katla/board_full.jpeg")) {
        std::cout << "Error: Gagal memuat file dari C:/Katla/Katla/" << std::endl;
        return -1;
    }

    sf::Sprite spriteLogo(texLogo);
    sf::Sprite spriteBoardFull(texBoardFull);

    // =========================================================================
    // KALIBRASI — dihitung dari resolusi asli:
    //   Logo  asli : 1080 x 282 px
    //   Board asli : 1080 x 690 px
    //   Window     : 600  x 900 px
    //
    // Rumus scale  : target_lebar / lebar_asli
    //
    // Jika ingin mengubah ukuran:
    //   LOGO_SCALE  → naikkan = logo lebih besar, turunkan = lebih kecil
    //   BOARD_SCALE → naikkan = board lebih besar, turunkan = lebih kecil
    //
    // Jika ingin mengubah posisi naik/turun:
    //   LOGO_Y  → angka kecil = naik, angka besar = turun
    //   BOARD_Y → angka kecil = naik, angka besar = turun
    //
    // Posisi kiri-kanan OTOMATIS center, tidak perlu diubah.
    // =========================================================================

    // Logo  : target lebar 480px  →  480 / 1080 = 0.444
    // Hasil : 480 x 125 px di layar
    const float LOGO_SCALE = 0.666f;   // <-- ubah jika logo perlu lebih besar/kecil
    const float LOGO_Y = 30.f;     // <-- ubah jika logo perlu naik/turun

    // Board : target lebar 520px  →  520 / 1080 = 0.481
    // Hasil : 520 x 332 px di layar
    const float BOARD_SCALE = 0.500f;  // <-- ubah jika board perlu lebih besar/kecil
    const float BOARD_Y = 230.f;   // <-- ubah jika board perlu naik/turun

    // =========================================================================

    // Terapkan logo
    spriteLogo.setScale({ LOGO_SCALE, LOGO_SCALE });
    spriteLogo.setPosition({
        300.f - (1080.f * LOGO_SCALE) / 2.f,
        LOGO_Y
        });

    // Terapkan board
    spriteBoardFull.setScale({ BOARD_SCALE, BOARD_SCALE });
    spriteBoardFull.setPosition({
        300.f - (1080.f * BOARD_SCALE) / 2.f,
        BOARD_Y
        });

    // Tombol otomatis di bawah board
    // Tinggi board di layar = 690 * 0.481 = ~332px
    // BOARD_Y(175) + 332 = 507 → tombol mulai di ~540
    const float BTN_W = 280.f;
    const float BTN_H = 58.f;
    const float BTN_GAP = 14.f;
    const float GAP_BOARD_BTN = 60.f;  // <-- jarak board ke tombol PLAY

    float boardBottomY = BOARD_Y + (690.f * BOARD_SCALE);
    float btnPlayY = boardBottomY + GAP_BOARD_BTN;
    float btnQuitY = btnPlayY + BTN_H + BTN_GAP;
    float btnX = 300.f - BTN_W / 2.f;

    sf::FloatRect rectBtnPlay({ btnX, btnPlayY }, { BTN_W, BTN_H });
    sf::FloatRect rectBtnQuit({ btnX, btnQuitY }, { BTN_W, BTN_H });

    // =========================================================================
    GameState currentState = GameState::MENU;
    GameOptions options;
    sf::Clock gameClock;

    std::vector<std::string> bankKata = {
        "ABANG", "ACARA", "AKTIF", "AGAMA", "AKHIR", "ALAMI", "AMBIL", "ANGIN", "BAGAN", "BAHAN",
        "BAKAR", "BALIK", "BATAS", "BEBAS", "BENAR", "BERAT", "BESAR", "BUKTI", "BULAN", "BUNGA",
        "CEPAT", "CERIA", "DALAM", "DAMAI", "DUNIA", "FAKTA", "FOKUS", "GAJAH", "GITAR", "HIDUP",
        "HIJAU", "HITAM", "HUJAN", "HURUF", "INDAH", "INGAT", "INGIN", "JALAN", "JARAK", "JATUH",
        "KAMAR", "KANAN", "KAPAL", "KASIH", "KAYAK", "KECIL", "KERJA", "KISAH", "KOTAK", "KUASA",
        "LAMPU", "LANGIT", "LEBIH", "LURUS", "MAKAN", "MALAM", "MANIS", "MASUK", "MERAH", "MIMPI",
        "MUDAH", "MULAI", "MUSIK", "NYATA", "PAGAR", "PAHAM", "PAKAI", "PANAS", "PASAR", "POHON",
        "PULAU", "PUTIH", "RINDU", "RUMAH", "SABTU", "SALAH", "SEHAT", "SEJAK", "SENJA", "SIANG",
        "SUARA", "SUDAH", "TAHUN", "TANAH", "TANYA", "TEPAT", "TIDAK", "TIDUR", "TUGAS", "TULIS",
        "UDARA", "UTAMA", "WAJAH", "WAKTU", "WARNA", "YAKIN", "ZAMAN"
    };

    std::srand(static_cast<unsigned int>(std::time(0)));
    std::string kataRahasia;
    std::vector<std::string> daftarTebakan;
    std::string tebakanSekarang = "";
    bool menang = false;
    bool kalah = false;

    while (window.isOpen()) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (currentState == GameState::MENU) {
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {
                        if (rectBtnPlay.contains(mousePos)) currentState = GameState::DIFFICULTY_SELECT;
                        if (rectBtnQuit.contains(mousePos)) window.close();
                    }
                }
            }
            else if (currentState == GameState::DIFFICULTY_SELECT) {
                if (const auto* mouseEvent = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseEvent->button == sf::Mouse::Button::Left) {
                        if (sf::FloatRect({ 150.f, 400.f }, { 300.f, 60.f }).contains(mousePos)) {
                            options.isHardMode = false;
                            kataRahasia = bankKata[std::rand() % bankKata.size()];
                            currentState = GameState::PLAYING;
                            menang = false; kalah = false; daftarTebakan.clear(); tebakanSekarang = "";
                        }
                        if (sf::FloatRect({ 150.f, 500.f }, { 300.f, 60.f }).contains(mousePos)) {
                            options.isHardMode = true;
                            options.timeLimit = 60.0f;
                            gameClock.restart();
                            kataRahasia = bankKata[std::rand() % bankKata.size()];
                            currentState = GameState::PLAYING;
                            menang = false; kalah = false; daftarTebakan.clear(); tebakanSekarang = "";
                        }
                    }
                }
            }
            else if (currentState == GameState::PLAYING) {
                if (!menang && !kalah) {
                    if (const auto* textEvent = event->getIf<sf::Event::TextEntered>()) {
                        if (textEvent->unicode < 128) {
                            char huruf = static_cast<char>(textEvent->unicode);
                            if (huruf == '\b' && !tebakanSekarang.empty()) tebakanSekarang.pop_back();
                            else if ((huruf == '\r' || huruf == '\n') && tebakanSekarang.size() == 5) {
                                daftarTebakan.push_back(tebakanSekarang);
                                if (tebakanSekarang == kataRahasia) menang = true;
                                else if (daftarTebakan.size() >= 6) kalah = true;
                                tebakanSekarang = "";
                            }
                            else if (std::isalpha(huruf) && tebakanSekarang.size() < 5)
                                tebakanSekarang += std::toupper(huruf);
                        }
                    }
                }
                else {
                    if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                        if (keyEvent->code == sf::Keyboard::Key::Enter) currentState = GameState::MENU;
                    }
                }
            }
        }

        if (currentState == GameState::PLAYING && options.isHardMode && !menang && !kalah) {
            float dt = gameClock.restart().asSeconds();
            options.timeLimit -= dt;
            if (options.timeLimit <= 0) { options.timeLimit = 0; kalah = true; }
        }

        window.clear(sf::Color(5, 5, 5));

        if (currentState == GameState::MENU) {
            window.draw(spriteLogo);
            window.draw(spriteBoardFull);

            // Tombol PLAY
            sf::ConvexShape btnPlay = buatKotakMelengkung(BTN_W, BTN_H, 15.f, sf::Color(65, 67, 70));
            btnPlay.setPosition({ btnX, btnPlayY });
            if (rectBtnPlay.contains(mousePos)) btnPlay.setFillColor(sf::Color(95, 97, 100));
            window.draw(btnPlay);

            sf::Text txtPlay(font, "PLAY", 26);
            txtPlay.setFillColor(sf::Color(230, 230, 230));
            txtPlay.setStyle(sf::Text::Bold);
            sf::FloatRect bP = txtPlay.getLocalBounds();
            txtPlay.setPosition({
                300.f - bP.size.x / 2.f - bP.position.x,
                (btnPlayY + BTN_H / 2.f) - bP.size.y / 2.f - bP.position.y
                });
            window.draw(txtPlay);

            // Tombol QUIT
            sf::ConvexShape btnQuit = buatKotakMelengkung(BTN_W, BTN_H, 15.f, sf::Color(100, 12, 24));
            btnQuit.setPosition({ btnX, btnQuitY });
            if (rectBtnQuit.contains(mousePos)) btnQuit.setFillColor(sf::Color(130, 15, 30));
            window.draw(btnQuit);

            sf::Text txtQuit(font, "QUIT", 26);
            txtQuit.setFillColor(sf::Color(230, 230, 230));
            txtQuit.setStyle(sf::Text::Bold);
            sf::FloatRect bQ = txtQuit.getLocalBounds();
            txtQuit.setPosition({
                300.f - bQ.size.x / 2.f - bQ.position.x,
                (btnQuitY + BTN_H / 2.f) - bQ.size.y / 2.f - bQ.position.y
                });
            window.draw(txtQuit);
        }
        else {
            sf::Text smallTitle(font, "KATLA", 50);
            smallTitle.setFillColor(sf::Color::White);
            sf::FloatRect bST = smallTitle.getLocalBounds();
            smallTitle.setPosition({ 300.f - bST.size.x / 2.f, 30.f });
            window.draw(smallTitle);

            if (currentState == GameState::DIFFICULTY_SELECT) {
                sf::Text sub(font, "PILIH MODE", 40);
                sf::FloatRect bSub = sub.getLocalBounds();
                sub.setPosition({ 300.f - bSub.size.x / 2.f, 250.f });
                window.draw(sub);

                sf::RectangleShape b1({ 300.f, 60.f }); b1.setPosition({ 150.f, 400.f }); b1.setFillColor(sf::Color(83, 141, 78));
                window.draw(b1);
                sf::Text t1(font, "NORMAL MODE", 25); sf::FloatRect bT1 = t1.getLocalBounds();
                t1.setPosition({ 300.f - bT1.size.x / 2.f - bT1.position.x, 430.f - bT1.size.y / 2.f - bT1.position.y });
                window.draw(t1);

                sf::RectangleShape b2({ 300.f, 60.f }); b2.setPosition({ 150.f, 500.f }); b2.setFillColor(sf::Color(181, 159, 59));
                window.draw(b2);
                sf::Text t2(font, "HARD MODE (1M)", 25); sf::FloatRect bT2 = t2.getLocalBounds();
                t2.setPosition({ 300.f - bT2.size.x / 2.f - bT2.position.x, 530.f - bT2.size.y / 2.f - bT2.position.y });
                window.draw(t2);
            }
            else if (currentState == GameState::PLAYING) {
                if (options.isHardMode && !menang && !kalah) {
                    int menit = (int)options.timeLimit / 60;
                    int detik = (int)options.timeLimit % 60;
                    std::string sWaktu = std::to_string(menit) + ":" + (detik < 10 ? "0" : "") + std::to_string(detik);
                    sf::Text tWaktu(font, sWaktu, 30); tWaktu.setPosition({ 480.f, 45.f });
                    window.draw(tWaktu);
                }

                for (int r = 0; r < 6; r++) {
                    std::vector<sf::Color> warnaBaris;
                    if (r < (int)daftarTebakan.size())
                        warnaBaris = hitungWarnaBaris(daftarTebakan[r], kataRahasia);
                    for (int c = 0; c < 5; c++) {
                        float x = 90.f + (c * 85.f);
                        float y = 170.f + (r * 85.f);
                        sf::RectangleShape box({ 70.f, 70.f });
                        box.setPosition({ x, y }); box.setOutlineThickness(2.f);
                        if (r < (int)daftarTebakan.size()) {
                            box.setFillColor(warnaBaris[c]); box.setOutlineColor(warnaBaris[c]);
                        }
                        else {
                            box.setFillColor(sf::Color::Transparent); box.setOutlineColor(sf::Color(58, 58, 60));
                        }
                        window.draw(box);

                        std::string sH = "";
                        if (r < (int)daftarTebakan.size()) sH = std::string(1, daftarTebakan[r][c]);
                        else if (r == (int)daftarTebakan.size() && c < (int)tebakanSekarang.size())
                            sH = std::string(1, tebakanSekarang[c]);

                        if (!sH.empty()) {
                            sf::Text t(font, sH, 40); t.setFillColor(sf::Color::White);
                            sf::FloatRect b = t.getLocalBounds();
                            t.setPosition({ x + 35.f - b.size.x / 2.f - b.position.x, y + 35.f - b.size.y / 2.f - b.position.y });
                            window.draw(t);
                        }
                    }
                }

                if (menang || kalah) {
                    sf::RectangleShape overlay({ 600.f, 900.f });
                    overlay.setFillColor(sf::Color(0, 0, 0, 220));
                    window.draw(overlay);

                    sf::Text msgMain(font, menang ? "YOU WIN" : "GAME OVER", 60);
                    msgMain.setFillColor(menang ? sf::Color::Yellow : sf::Color::Red);
                    sf::FloatRect bM = msgMain.getLocalBounds();
                    msgMain.setPosition({ 300.f - bM.size.x / 2.f, 350.f });
                    window.draw(msgMain);

                    sf::Text tJawab(font, "Kata kunci: " + kataRahasia, 25);
                    tJawab.setFillColor(sf::Color::White);
                    sf::FloatRect bJ = tJawab.getLocalBounds();
                    tJawab.setPosition({ 300.f - bJ.size.x / 2.f, 450.f });
                    window.draw(tJawab);

                    sf::Text tBack(font, "Tekan ENTER untuk kembali ke menu", 18);
                    tBack.setFillColor(sf::Color(180, 180, 180));
                    sf::FloatRect bBack = tBack.getLocalBounds();
                    tBack.setPosition({ 300.f - bBack.size.x / 2.f, 520.f });
                    window.draw(tBack);
                }
            }
        }

        window.display();
    }
    return 0;
}