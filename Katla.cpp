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
    float timeLimit = 180.0f; // 3 menit = 180 detik
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

    // =========================================================================
    // Load semua texture
    // =========================================================================
    sf::Texture texLogo, texBoardFull, texMode;

    if (!texLogo.loadFromFile("C:/Katla/Katla/logo.jpeg") ||
        !texBoardFull.loadFromFile("C:/Katla/Katla/board_full.jpeg") ||
        !texMode.loadFromFile("C:/Katla/Katla/mode.png")) {
        std::cout << "Error: Gagal memuat file dari C:/Katla/Katla/" << std::endl;
        return -1;
    }

    sf::Sprite spriteLogo(texLogo);
    sf::Sprite spriteBoardFull(texBoardFull);
    sf::Sprite spriteMode(texMode);

    // =========================================================================
    // KALIBRASI MENU
    // =========================================================================
    const float LOGO_SCALE = 0.666f;
    const float LOGO_Y = 30.f;
    const float BOARD_SCALE = 0.500f;
    const float BOARD_Y = 230.f;

    spriteLogo.setScale({ LOGO_SCALE, LOGO_SCALE });
    spriteLogo.setPosition({
        300.f - (1080.f * LOGO_SCALE) / 2.f,
        LOGO_Y
        });

    spriteBoardFull.setScale({ BOARD_SCALE, BOARD_SCALE });
    spriteBoardFull.setPosition({
        300.f - (1080.f * BOARD_SCALE) / 2.f,
        BOARD_Y
        });

    // =========================================================================
    // KALIBRASI MODE SCREEN — gambar mode.png
    //   MODE_SCALE → ubah ukuran gambar mode (1.0 = lebar penuh 600px)
    //   MODE_Y     → geser naik/turun
    // =========================================================================
    const float MODE_SCALE = 1.0f;
    const float MODE_Y = 200.f;

    {
        sf::Vector2u modeSize = texMode.getSize();
        float scaleX = 600.f / static_cast<float>(modeSize.x);
        float autoScale = scaleX * MODE_SCALE;
        spriteMode.setScale({ autoScale, autoScale });
        spriteMode.setPosition({
            300.f - (static_cast<float>(modeSize.x) * autoScale) / 2.f,
            MODE_Y
            });
    }

    // =========================================================================
    // KALIBRASI LOGO DI DIFFICULTY SELECT SCREEN
    //   Sprite terpisah dari spriteLogo menu agar scale/posisi bisa beda.
    //
    //   MODE_LOGO_SCALE → ukuran logo (0.45 ≈ lebar 216px dari 1080px asli)
    //   MODE_LOGO_Y     → jarak dari atas layar
    //                     angka kecil = naik, angka besar = turun
    // Logo otomatis di-center horizontal.
    // =========================================================================
    const float MODE_LOGO_SCALE = 0.45f;   // <-- ubah jika logo perlu lebih besar/kecil
    const float MODE_LOGO_Y = 45.f;    // <-- ubah jika logo perlu naik/turun

    sf::Sprite spriteModeLogo(texLogo);
    spriteModeLogo.setScale({ MODE_LOGO_SCALE, MODE_LOGO_SCALE });
    spriteModeLogo.setPosition({
        300.f - (1080.f * MODE_LOGO_SCALE) / 2.f,
        MODE_LOGO_Y
        });

    // =========================================================================
    // Tombol MENU (PLAY & QUIT)
    // =========================================================================
    const float BTN_W = 280.f;
    const float BTN_H = 58.f;
    const float BTN_GAP = 14.f;
    const float GAP_BOARD_BTN = 60.f;

    float boardBottomY = BOARD_Y + (690.f * BOARD_SCALE);
    float btnPlayY = boardBottomY + GAP_BOARD_BTN;
    float btnQuitY = btnPlayY + BTN_H + BTN_GAP;
    float btnX = 300.f - BTN_W / 2.f;

    sf::FloatRect rectBtnPlay({ btnX, btnPlayY }, { BTN_W, BTN_H });
    sf::FloatRect rectBtnQuit({ btnX, btnQuitY }, { BTN_W, BTN_H });

    // =========================================================================
    // Tombol DIFFICULTY SELECT
    //   DBTN_NORMAL_Y / DBTN_HARD_Y → geser agar tombol pas di atas mode.png
    // =========================================================================
    const float DBTN_W = 310.f;
    const float DBTN_H = 62.f;
    const float DBTN_X = 300.f - DBTN_W / 2.f;
    const float DBTN_NORMAL_Y = 490.f;   // <-- geser jika perlu
    const float DBTN_HARD_Y = 590.f;   // <-- geser jika perlu

    sf::FloatRect rectBtnNormal({ DBTN_X, DBTN_NORMAL_Y }, { DBTN_W, DBTN_H });
    sf::FloatRect rectBtnHard({ DBTN_X, DBTN_HARD_Y }, { DBTN_W, DBTN_H });

    // =========================================================================
    GameState   currentState = GameState::MENU;
    GameOptions options;
    sf::Clock   gameClock;

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
    std::string              kataRahasia;
    std::vector<std::string> daftarTebakan;
    std::string              tebakanSekarang = "";
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
                        if (rectBtnNormal.contains(mousePos)) {
                            options.isHardMode = false;
                            kataRahasia = bankKata[std::rand() % bankKata.size()];
                            currentState = GameState::PLAYING;
                            menang = false; kalah = false;
                            daftarTebakan.clear(); tebakanSekarang = "";
                        }
                        if (rectBtnHard.contains(mousePos)) {
                            options.isHardMode = true;
                            options.timeLimit = 180.0f;
                            gameClock.restart();
                            kataRahasia = bankKata[std::rand() % bankKata.size()];
                            currentState = GameState::PLAYING;
                            menang = false; kalah = false;
                            daftarTebakan.clear(); tebakanSekarang = "";
                        }
                    }
                }
                if (const auto* keyEvent = event->getIf<sf::Event::KeyPressed>()) {
                    if (keyEvent->code == sf::Keyboard::Key::Escape)
                        currentState = GameState::MENU;
                }
            }
            else if (currentState == GameState::PLAYING) {
                if (!menang && !kalah) {
                    if (const auto* textEvent = event->getIf<sf::Event::TextEntered>()) {
                        if (textEvent->unicode < 128) {
                            char huruf = static_cast<char>(textEvent->unicode);
                            if (huruf == '\b' && !tebakanSekarang.empty())
                                tebakanSekarang.pop_back();
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
                        if (keyEvent->code == sf::Keyboard::Key::Enter)
                            currentState = GameState::MENU;
                    }
                }
            }
        }

        // Update timer Hard Mode
        if (currentState == GameState::PLAYING && options.isHardMode && !menang && !kalah) {
            float dt = gameClock.restart().asSeconds();
            options.timeLimit -= dt;
            if (options.timeLimit <= 0) { options.timeLimit = 0; kalah = true; }
        }

        // =====================================================================
        // RENDER
        // =====================================================================
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
        else if (currentState == GameState::DIFFICULTY_SELECT) {
            // 1. Background mode.png
            window.draw(spriteMode);

            // 2. Logo Katka di atas mode.png
            //    Digambar setelah mode agar tampil di depan background
            window.draw(spriteModeLogo);

            // 3. Tombol NORMAL MODE (abu-abu rounded)
            sf::ConvexShape btnNormal = buatKotakMelengkung(DBTN_W, DBTN_H, 20.f, sf::Color(90, 92, 96));
            btnNormal.setPosition({ DBTN_X, DBTN_NORMAL_Y });
            if (rectBtnNormal.contains(mousePos)) btnNormal.setFillColor(sf::Color(115, 117, 122));
            window.draw(btnNormal);

            sf::Text tNormal(font, "NORMAL MODE", 26);
            tNormal.setFillColor(sf::Color(230, 230, 230));
            tNormal.setStyle(sf::Text::Bold);
            sf::FloatRect bN = tNormal.getLocalBounds();
            tNormal.setPosition({
                300.f - bN.size.x / 2.f - bN.position.x,
                (DBTN_NORMAL_Y + DBTN_H / 2.f) - bN.size.y / 2.f - bN.position.y
                });
            window.draw(tNormal);

            // 4. Tombol HARD MODE (merah tua rounded)
            sf::ConvexShape btnHard = buatKotakMelengkung(DBTN_W, DBTN_H, 20.f, sf::Color(120, 20, 35));
            btnHard.setPosition({ DBTN_X, DBTN_HARD_Y });
            if (rectBtnHard.contains(mousePos)) btnHard.setFillColor(sf::Color(150, 28, 45));
            window.draw(btnHard);

            sf::Text tHard(font, "HARD MODE (3M)", 26);
            tHard.setFillColor(sf::Color(230, 230, 230));
            tHard.setStyle(sf::Text::Bold);
            sf::FloatRect bH = tHard.getLocalBounds();
            tHard.setPosition({
                300.f - bH.size.x / 2.f - bH.position.x,
                (DBTN_HARD_Y + DBTN_H / 2.f) - bH.size.y / 2.f - bH.position.y
                });
            window.draw(tHard);
        }
        else if (currentState == GameState::PLAYING) {
            // Judul kecil
            sf::Text smallTitle(font, "KATLA", 50);
            smallTitle.setFillColor(sf::Color::White);
            sf::FloatRect bST = smallTitle.getLocalBounds();
            smallTitle.setPosition({ 300.f - bST.size.x / 2.f, 30.f });
            window.draw(smallTitle);

            // Timer Hard Mode
            if (options.isHardMode && !menang && !kalah) {
                int menit = (int)options.timeLimit / 60;
                int detik = (int)options.timeLimit % 60;
                std::string sWaktu = std::to_string(menit) + ":" + (detik < 10 ? "0" : "") + std::to_string(detik);
                sf::Text tWaktu(font, sWaktu, 30);
                tWaktu.setFillColor(options.timeLimit <= 30.f ? sf::Color::Red : sf::Color::White);
                tWaktu.setPosition({ 480.f, 45.f });
                window.draw(tWaktu);
            }

            // Grid tebakan
            for (int r = 0; r < 6; r++) {
                std::vector<sf::Color> warnaBaris;
                if (r < (int)daftarTebakan.size())
                    warnaBaris = hitungWarnaBaris(daftarTebakan[r], kataRahasia);

                for (int c = 0; c < 5; c++) {
                    float x = 90.f + (c * 85.f);
                    float y = 170.f + (r * 85.f);

                    sf::RectangleShape box({ 70.f, 70.f });
                    box.setPosition({ x, y });
                    box.setOutlineThickness(2.f);

                    if (r < (int)daftarTebakan.size()) {
                        box.setFillColor(warnaBaris[c]);
                        box.setOutlineColor(warnaBaris[c]);
                    }
                    else {
                        box.setFillColor(sf::Color::Transparent);
                        box.setOutlineColor(sf::Color(58, 58, 60));
                    }
                    window.draw(box);

                    std::string sH = "";
                    if (r < (int)daftarTebakan.size())
                        sH = std::string(1, daftarTebakan[r][c]);
                    else if (r == (int)daftarTebakan.size() && c < (int)tebakanSekarang.size())
                        sH = std::string(1, tebakanSekarang[c]);

                    if (!sH.empty()) {
                        sf::Text t(font, sH, 40);
                        t.setFillColor(sf::Color::White);
                        sf::FloatRect b = t.getLocalBounds();
                        t.setPosition({
                            x + 35.f - b.size.x / 2.f - b.position.x,
                            y + 35.f - b.size.y / 2.f - b.position.y
                            });
                        window.draw(t);
                    }
                }
            }

            // Overlay menang / kalah
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

        window.display();
    }
    return 0;
}
