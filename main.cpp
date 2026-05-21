#include <SFML/Graphics.hpp>
#include <optional>
#include <vector>
#include <string>
#include <ctime>
#include <cstdlib>
#include <algorithm>
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

// =============================================================================
// STATE
// =============================================================================
enum class GameState { MENU, INPUT_NAMA, DIFFICULTY_SELECT, PLAYING, RESULT };

struct GameOptions {
    bool  isHardMode = false;
    float timeLimit = 180.0f;
};

// =============================================================================
// HELPER: Rounded box
// =============================================================================
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

// =============================================================================
// HELPER: Format waktu mm:ss
// =============================================================================
std::string formatWaktu(float detikTotal) {
    if (detikTotal < 0.f) detikTotal = 0.f;
    int menit = (int)detikTotal / 60;
    int detik = (int)detikTotal % 60;
    std::ostringstream oss;
    oss << menit << ":" << std::setw(2) << std::setfill('0') << detik;
    return oss.str();
}

// =============================================================================
// HELPER: Hitung warna baris Wordle
// =============================================================================
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

// =============================================================================
// HELPER: Gambar teks center horizontal pada Y tertentu
// =============================================================================
void drawCenteredText(sf::RenderWindow& win, sf::Text& txt, float y) {
    sf::FloatRect b = txt.getLocalBounds();
    txt.setPosition({ 300.f - b.size.x / 2.f - b.position.x, y });
    win.draw(txt);
}

// =============================================================================
// HELPER: Gambar ikon jam analog (SFML shapes) di posisi cx, cy radius r
// =============================================================================
void drawClockIcon(sf::RenderWindow& win, float cx, float cy, float r, sf::Color col) {
    sf::CircleShape outer(r);
    outer.setFillColor(sf::Color::Transparent);
    outer.setOutlineThickness(2.5f);
    outer.setOutlineColor(col);
    outer.setPosition({ cx - r, cy - r });
    win.draw(outer);

    const float PI = 3.14159265f;
    float angJam = -60.f * PI / 180.f;
    float angMen = 60.f * PI / 180.f;

    auto drawHand = [&](float angle, float length, float thickness, sf::Color c) {
        sf::RectangleShape hand({ thickness, length });
        hand.setFillColor(c);
        hand.setOrigin({ thickness / 2.f, length });
        hand.setPosition({ cx, cy });
        hand.setRotation(sf::degrees(angle * 180.f / PI));
        win.draw(hand);
        };

    drawHand(angJam, r * 0.55f, 2.5f, col);
    drawHand(angMen, r * 0.75f, 2.0f, col);

    sf::CircleShape dot(2.5f);
    dot.setFillColor(col);
    dot.setPosition({ cx - 2.5f, cy - 2.5f });
    win.draw(dot);
}

// =============================================================================
// MAIN
// =============================================================================
int main() {
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(600, 900)), "Katla - Tebak Kata");
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("arial.ttf")) {
        if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) return -1;
    }

    // -------------------------------------------------------------------------
    // Textures
    // -------------------------------------------------------------------------
    sf::Texture texLogo, texBoardFull, texMode, texGameOver, texYouWin;
    if (!texLogo.loadFromFile("C:/Katla/Katla/logo.jpeg") ||
        !texBoardFull.loadFromFile("C:/Katla/Katla/board_full.jpeg") ||
        !texMode.loadFromFile("C:/Katla/Katla/mode.png") ||
        !texGameOver.loadFromFile("C:/Katla/Katla/game_over.png") ||
        !texYouWin.loadFromFile("C:/Katla/Katla/youwin.png")) {
        std::cout << "Error: Gagal memuat file dari C:/Katla/Katla/" << std::endl;
        return -1;
    }

    // -------------------------------------------------------------------------
    // Sprites MENU
    // -------------------------------------------------------------------------
    sf::Sprite spriteLogo(texLogo);
    sf::Sprite spriteBoardFull(texBoardFull);
    sf::Sprite spriteMode(texMode);

    const float LOGO_SCALE = 0.666f; const float LOGO_Y = 30.f;
    const float BOARD_SCALE = 0.500f; const float BOARD_Y = 230.f;

    spriteLogo.setScale({ LOGO_SCALE, LOGO_SCALE });
    spriteLogo.setPosition({ 300.f - (1080.f * LOGO_SCALE) / 2.f, LOGO_Y });
    spriteBoardFull.setScale({ BOARD_SCALE, BOARD_SCALE });
    spriteBoardFull.setPosition({ 300.f - (1080.f * BOARD_SCALE) / 2.f, BOARD_Y });

    // mode.png
    const float MODE_SCALE = 1.0f;
    const float MODE_Y = 200.f;
    {
        sf::Vector2u sz = texMode.getSize();
        float sc = (600.f / static_cast<float>(sz.x)) * MODE_SCALE;
        spriteMode.setScale({ sc, sc });
        spriteMode.setPosition({ 300.f - (sz.x * sc) / 2.f, MODE_Y });
    }

    // Logo kecil di difficulty / input nama screen
    const float MODE_LOGO_SCALE = 0.45f;
    const float MODE_LOGO_Y = 30.f;
    sf::Sprite spriteModeLogo(texLogo);
    spriteModeLogo.setScale({ MODE_LOGO_SCALE, MODE_LOGO_SCALE });
    spriteModeLogo.setPosition({ 300.f - (1080.f * MODE_LOGO_SCALE) / 2.f, MODE_LOGO_Y });

    // -------------------------------------------------------------------------
    // Sprite logo halaman PLAYING
    // -------------------------------------------------------------------------
    const float PLAY_LOGO_SCALE = 0.5f;
    const float PLAY_LOGO_Y = 8.f;
    sf::Sprite spritePlayLogo(texLogo);
    spritePlayLogo.setScale({ PLAY_LOGO_SCALE, PLAY_LOGO_SCALE });
    spritePlayLogo.setPosition({ 300.f - (1080.f * PLAY_LOGO_SCALE) / 2.f, PLAY_LOGO_Y });

    // =========================================================================
    // Sprites RESULT  — YOU WIN & GAME OVER (konstanta TERPISAH)
    // =========================================================================
    //  YOU WIN  — ubah 2 baris ini untuk mengatur gambar You Win:
    const float YOUWIN_IMG_W = 530.f;   // ← lebar gambar You Win (px), maks ~600
    const float YOUWIN_IMG_Y = 75.f;    // ← posisi Y You Win (kecil=naik, besar=turun)

    //  GAME OVER — ubah 2 baris ini untuk mengatur gambar Game Over:
    const float GAMEOVER_IMG_W = 850.f; // ← lebar gambar Game Over (px), maks ~600
    const float GAMEOVER_IMG_Y = 40.f;  // ← posisi Y Game Over (kecil=naik, besar=turun)
    // =========================================================================

    sf::Sprite spriteYouWin(texYouWin);
    {
        sf::Vector2u sz = texYouWin.getSize();
        float sc = YOUWIN_IMG_W / static_cast<float>(sz.x);
        spriteYouWin.setScale({ sc, sc });
        spriteYouWin.setPosition({ 300.f - (sz.x * sc) / 2.f, YOUWIN_IMG_Y });
    }

    sf::Sprite spriteGameOver(texGameOver);
    {
        sf::Vector2u sz = texGameOver.getSize();
        float sc = GAMEOVER_IMG_W / static_cast<float>(sz.x);
        spriteGameOver.setScale({ sc, sc });
        spriteGameOver.setPosition({ 300.f - (sz.x * sc) / 2.f, GAMEOVER_IMG_Y });
    }

    // -------------------------------------------------------------------------
    // Tombol MENU
    // -------------------------------------------------------------------------
    const float BTN_W = 280.f, BTN_H = 58.f, BTN_GAP = 14.f;
    float boardBottomY = BOARD_Y + (690.f * BOARD_SCALE);
    float btnPlayY = boardBottomY + 60.f;
    float btnQuitY = btnPlayY + BTN_H + BTN_GAP;
    float btnX = 300.f - BTN_W / 2.f;
    sf::FloatRect rectBtnPlay({ btnX, btnPlayY }, { BTN_W, BTN_H });
    sf::FloatRect rectBtnQuit({ btnX, btnQuitY }, { BTN_W, BTN_H });

    // -------------------------------------------------------------------------
    // Tombol DIFFICULTY SELECT
    // -------------------------------------------------------------------------
    const float DBTN_W = 310.f, DBTN_H = 62.f;
    const float DBTN_X = 300.f - DBTN_W / 2.f;
    const float DBTN_NORMAL_Y = 490.f;
    const float DBTN_HARD_Y = 590.f;
    sf::FloatRect rectBtnNormal({ DBTN_X, DBTN_NORMAL_Y }, { DBTN_W, DBTN_H });
    sf::FloatRect rectBtnHard({ DBTN_X, DBTN_HARD_Y }, { DBTN_W, DBTN_H });

    // =========================================================================
    // Layout RESULT — panel info + tombol
    // =========================================================================
    const float RESULT_PANEL_Y = 300.f;
    const float RESULT_PANEL_H = 200.f;
    const float RESULT_PANEL_PAD = 56.f;
    const float RESULT_BTN_W = 300.f;
    const float RESULT_BTN_H = 55.f;
    const float RESULT_BTN_GAP = 10.f;
    const float RESULT_BTN_X = 300.f - RESULT_BTN_W / 2.f;
    float RESULT_BTN_Y1 = RESULT_PANEL_Y + RESULT_PANEL_H + RESULT_PANEL_PAD;
    float RESULT_BTN_Y2 = RESULT_BTN_Y1 + RESULT_BTN_H + RESULT_BTN_GAP;

    sf::FloatRect rectBtnPlayAgain({ RESULT_BTN_X, RESULT_BTN_Y1 }, { RESULT_BTN_W, RESULT_BTN_H });
    sf::FloatRect rectBtnBackMenu({ RESULT_BTN_X, RESULT_BTN_Y2 }, { RESULT_BTN_W, RESULT_BTN_H });

    // -------------------------------------------------------------------------
    // Game state
    // -------------------------------------------------------------------------
    GameState   currentState = GameState::MENU;
    GameOptions options;

    sf::Clock stopwatchClock;
    sf::Clock hardClock;

    std::vector<std::string> bankKata = {
        // A
        "ABADI", "ABANG", "ABRIK", "ACARA", "ADUAN", "AGAMA", "AKHIR", "AKRAB",
        "AKTIF", "ALAMI", "ALANG", "AMBIL", "AMBAN", "ANGIN", "ANGKA", "ANTAP",
        // B
        "BABAT", "BAHAN", "BAHAS", "BAKAR", "BALIK", "BARAT", "BATAS", "BATUK",
        "BAYAM", "BEBAS", "BEKAL", "BENAR", "BERAT", "BESAR", "BIASA", "BUBUR",
        "BULAN", "BUNGA", "BURUH", "BURUK",
        // C
        "CAKAP", "CALAK", "CAMUR", "CANDA", "CAPAT", "CEPAT", "CERIA", "CORAK",
        // D
        "DADAR", "DALAM", "DAMAI", "DAPUR", "DARAH", "DATUK", "DAUNG", "DAWAI",
        "DEBAT", "DEKAT", "DENAH", "DERAS", "DEWAN", "DOKAR", "DUNIA", "DURJA",
        // F
        "FAJAR", "FAKTA", "FASIH", "FIKIR", "FIKRI", "FISIK", "FOKUS",
        // G
        "GAJAH", "GALAK", "GAMAK", "GAMAT", "GAUNG", "GIGIH", "GITAR", "GUGUR",
        // H
        "HABIS", "HALUS", "HARAP", "HEMAT", "HIDUP", "HIJAU", "HITAM", "HUJAN",
        // I
        "IKHLAS", "ILAHI", "ILHAM", "IMBAL", "IMPAS", "INDAH", "INGAT", "INGIN",
        // J
        "JAJAN", "JALAN", "JARAK", "JATUH", "JAWAB", "JINAK", "JUARA",
        // K
        "KABAR", "KADAR", "KAMAR", "KANAN", "KAPAL", "KASIH", "KECIL", "KERAS",
        "KERJA", "KIDAL", "KISAH", "KOTAK", "KUASA", "KUBAH", "KUKUH", "KULAK",
        // L
        "LABUH", "LAHAN", "LAKAR", "LAMPU", "LANCAR","LAYAK", "LEBIH", "LEKAS",
        "LEMBAH","LEMAH", "LURUS",
        // M
        "MAKAN", "MALAM", "MANIS", "MASUK", "MATAP", "MEDAN", "MERAH", "MIMPI",
        "MUDAH", "MULAI", "MULIA", "MUSIK", "MUTAR",
        // N
        "NALAR", "NANAS", "NAPAS", "NYATA", "NYAWA",
        // P
        "PAGAR", "PAHAM", "PAKAI", "PANAS", "PASAR", "PATUH", "PERAN", "PERLU",
        "PILIH", "POHON", "POKOK", "PULAU", "PUTIH",
        // R
        "RAMAI", "RAWAT", "REZKI", "RIANG", "RINDU", "RISAU", "ROBOH", "RUANG",
        "RUMAH",
        // S
        "SABTU", "SABAR", "SADAR", "SALAH", "SEHAT", "SEJAK", "SENAM", "SENJA",
        "SIANG", "SIBUK", "SIGAP", "SIHAT", "SUBUR", "SUARA", "SUDAH", "SUNYI",
        // T
        "TABAH", "TAHUN", "TANAH", "TANYA", "TEGAK", "TEGAS", "TEPAT", "TIDAK",
        "TIDUR", "TULUS", "TUGAS", "TULIS",
        // U
        "UDARA", "ULUNG", "UTAMA", "UTARA",
        // W
        "WAJAH", "WAKTU", "WARGA", "WARNA", "WATAK",
        // Y
        "YAKIN",
        // Z
        "ZAMAN"
    };
    std::srand(static_cast<unsigned int>(std::time(0)));

    std::string              namaUser = "";
    std::string              kataRahasia = "";
    std::vector<std::string> daftarTebakan;
    std::string              tebakanSekarang = "";
    bool                     menang = false;
    float                    waktuTerpakai = 0.f;

    sf::Clock cursorClock;
    bool      cursorVisible = true;

    // =========================================================================
    // GAME LOOP
    // =========================================================================
    while (window.isOpen()) {
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        if (cursorClock.getElapsedTime().asSeconds() > 0.5f) {
            cursorVisible = !cursorVisible;
            cursorClock.restart();
        }

        // ====================================================================
        // EVENT
        // ====================================================================
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (currentState == GameState::MENU) {
                if (const auto* me = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (me->button == sf::Mouse::Button::Left) {
                        if (rectBtnPlay.contains(mousePos)) currentState = GameState::INPUT_NAMA;
                        if (rectBtnQuit.contains(mousePos)) window.close();
                    }
                }
            }
            else if (currentState == GameState::INPUT_NAMA) {
                if (const auto* te = event->getIf<sf::Event::TextEntered>()) {
                    if (te->unicode < 128) {
                        char c = static_cast<char>(te->unicode);
                        if (c == '\b' && !namaUser.empty())
                            namaUser.pop_back();
                        else if ((c == '\r' || c == '\n') && !namaUser.empty())
                            currentState = GameState::DIFFICULTY_SELECT;
                        else if (std::isprint(c) && namaUser.size() < 20)
                            namaUser += c;
                    }
                }
                if (const auto* ke = event->getIf<sf::Event::KeyPressed>())
                    if (ke->code == sf::Keyboard::Key::Escape)
                        currentState = GameState::MENU;
            }
            else if (currentState == GameState::DIFFICULTY_SELECT) {
                if (const auto* me = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (me->button == sf::Mouse::Button::Left) {
                        auto startGame = [&](bool hard) {
                            options.isHardMode = hard;
                            options.timeLimit = 60.0f;
                            kataRahasia = bankKata[std::rand() % bankKata.size()];
                            daftarTebakan.clear();
                            tebakanSekarang = "";
                            menang = false;
                            waktuTerpakai = 0.f;
                            stopwatchClock.restart();
                            hardClock.restart();
                            currentState = GameState::PLAYING;
                            };
                        if (rectBtnNormal.contains(mousePos)) startGame(false);
                        if (rectBtnHard.contains(mousePos))   startGame(true);
                    }
                }
                if (const auto* ke = event->getIf<sf::Event::KeyPressed>())
                    if (ke->code == sf::Keyboard::Key::Escape)
                        currentState = GameState::INPUT_NAMA;
            }
            else if (currentState == GameState::PLAYING) {
                if (const auto* te = event->getIf<sf::Event::TextEntered>()) {
                    if (te->unicode < 128) {
                        char c = static_cast<char>(te->unicode);
                        if (c == '\b' && !tebakanSekarang.empty())
                            tebakanSekarang.pop_back();
                        else if ((c == '\r' || c == '\n') && tebakanSekarang.size() == 5) {
                            daftarTebakan.push_back(tebakanSekarang);
                            bool selesai = false;
                            if (tebakanSekarang == kataRahasia) { menang = true;  selesai = true; }
                            else if (daftarTebakan.size() >= 6) { menang = false; selesai = true; }
                            tebakanSekarang = "";
                            if (selesai) {
                                waktuTerpakai = options.isHardMode
                                    ? 60.f - options.timeLimit
                                    : stopwatchClock.getElapsedTime().asSeconds();
                                currentState = GameState::RESULT;
                            }
                        }
                        else if (std::isalpha(c) && tebakanSekarang.size() < 5)
                            tebakanSekarang += std::toupper(c);
                    }
                }
            }
            else if (currentState == GameState::RESULT) {
                if (const auto* me = event->getIf<sf::Event::MouseButtonPressed>()) {
                    if (me->button == sf::Mouse::Button::Left) {
                        if (rectBtnPlayAgain.contains(mousePos)) {
                            options.timeLimit = 60.0f;
                            kataRahasia = bankKata[std::rand() % bankKata.size()];
                            daftarTebakan.clear();
                            tebakanSekarang = "";
                            menang = false;
                            waktuTerpakai = 0.f;
                            stopwatchClock.restart();
                            hardClock.restart();
                            currentState = GameState::PLAYING;
                        }
                        if (rectBtnBackMenu.contains(mousePos))
                            currentState = GameState::MENU;
                    }
                }
                if (const auto* ke = event->getIf<sf::Event::KeyPressed>())
                    if (ke->code == sf::Keyboard::Key::Enter)
                        currentState = GameState::MENU;
            }
        }

        // ====================================================================
        // UPDATE
        // ====================================================================
        if (currentState == GameState::PLAYING && options.isHardMode) {
            float dt = hardClock.restart().asSeconds();
            options.timeLimit -= dt;
            if (options.timeLimit <= 0.f) {
                options.timeLimit = 0.f;
                waktuTerpakai = 60.f;
                menang = false;
                currentState = GameState::RESULT;
            }
        }

        // ====================================================================
        // RENDER
        // ====================================================================
        window.clear(sf::Color(5, 5, 5));

        // --------------------------------------------------------------------
        // MENU
        // --------------------------------------------------------------------
        if (currentState == GameState::MENU) {
            window.draw(spriteLogo);
            window.draw(spriteBoardFull);

            auto drawMenuBtn = [&](sf::FloatRect r, sf::Color c, sf::Color h, const std::string& lbl) {
                sf::ConvexShape btn = buatKotakMelengkung(r.size.x, r.size.y, 15.f, c);
                btn.setPosition(r.position);
                if (r.contains(mousePos)) btn.setFillColor(h);
                window.draw(btn);
                sf::Text txt(font, lbl, 26);
                txt.setFillColor(sf::Color(230, 230, 230));
                txt.setStyle(sf::Text::Bold);
                sf::FloatRect b = txt.getLocalBounds();
                txt.setPosition({ 300.f - b.size.x / 2.f - b.position.x,
                                  r.position.y + r.size.y / 2.f - b.size.y / 2.f - b.position.y });
                window.draw(txt);
                };
            drawMenuBtn(rectBtnPlay, sf::Color(65, 67, 70), sf::Color(95, 97, 100), "PLAY");
            drawMenuBtn(rectBtnQuit, sf::Color(100, 12, 24), sf::Color(130, 15, 30), "QUIT");
        }

        // --------------------------------------------------------------------
        // INPUT NAMA
        // --------------------------------------------------------------------
        else if (currentState == GameState::INPUT_NAMA) {
            window.draw(spriteModeLogo);

            sf::ConvexShape panel = buatKotakMelengkung(460.f, 220.f, 20.f, sf::Color(30, 30, 32));
            panel.setPosition({ 70.f, 310.f });
            window.draw(panel);

            sf::Text tPrompt(font, "Masukkan nama kamu:", 24);
            tPrompt.setFillColor(sf::Color(200, 200, 200));
            drawCenteredText(window, tPrompt, 340.f);

            sf::ConvexShape inputBox = buatKotakMelengkung(380.f, 58.f, 12.f, sf::Color(50, 50, 55));
            inputBox.setPosition({ 110.f, 390.f });
            window.draw(inputBox);

            std::string tampilNama = namaUser + (cursorVisible ? "|" : " ");
            sf::Text tNama(font, tampilNama, 28);
            tNama.setFillColor(sf::Color::White);
            tNama.setStyle(sf::Text::Bold);
            drawCenteredText(window, tNama, 402.f);

            sf::Text tHint(font, "Tekan ENTER untuk lanjut", 18);
            tHint.setFillColor(sf::Color(120, 120, 120));
            drawCenteredText(window, tHint, 480.f);

            sf::Text tEsc(font, "ESC = kembali ke menu", 16);
            tEsc.setFillColor(sf::Color(80, 80, 80));
            drawCenteredText(window, tEsc, 510.f);
        }

        // --------------------------------------------------------------------
        // DIFFICULTY SELECT
        // --------------------------------------------------------------------
        else if (currentState == GameState::DIFFICULTY_SELECT) {
            window.draw(spriteMode);
            window.draw(spriteModeLogo);

            auto drawDBtn = [&](sf::FloatRect r, sf::Color c, sf::Color h, const std::string& lbl) {
                sf::ConvexShape btn = buatKotakMelengkung(r.size.x, r.size.y, 20.f, c);
                btn.setPosition(r.position);
                if (r.contains(mousePos)) btn.setFillColor(h);
                window.draw(btn);
                sf::Text txt(font, lbl, 26);
                txt.setFillColor(sf::Color(230, 230, 230));
                txt.setStyle(sf::Text::Bold);
                sf::FloatRect b = txt.getLocalBounds();
                txt.setPosition({ 300.f - b.size.x / 2.f - b.position.x,
                                  r.position.y + r.size.y / 2.f - b.size.y / 2.f - b.position.y });
                window.draw(txt);
                };
            drawDBtn(rectBtnNormal, sf::Color(90, 92, 96), sf::Color(115, 117, 122), "NORMAL MODE");
            drawDBtn(rectBtnHard, sf::Color(120, 20, 35), sf::Color(150, 28, 45), "HARD MODE");
        }

        // --------------------------------------------------------------------
        // PLAYING
        // --------------------------------------------------------------------
        else if (currentState == GameState::PLAYING) {
            window.draw(spritePlayLogo);

            sf::Text tNama(font, namaUser, 20);
            tNama.setFillColor(sf::Color(180, 180, 180));
            tNama.setPosition({ 20.f, 30.f });
            window.draw(tNama);

            float timerVal;
            sf::Color timerColor;
            if (options.isHardMode) {
                timerVal = options.timeLimit;
                timerColor = (timerVal <= 30.f) ? sf::Color::Red : sf::Color::White;
            }
            else {
                timerVal = stopwatchClock.getElapsedTime().asSeconds();
                timerColor = sf::Color(180, 180, 180);
            }
            sf::Text tWaktu(font, formatWaktu(timerVal), 26);
            tWaktu.setFillColor(timerColor);
            tWaktu.setStyle(options.isHardMode && timerVal <= 30.f ? sf::Text::Bold : sf::Text::Regular);
            sf::FloatRect bW = tWaktu.getLocalBounds();

            const float CLOCK_R = 14.f;
            const float GAP = 8.f;
            const float RIGHT_EDGE = 575.f;
            float clockCx = RIGHT_EDGE - CLOCK_R;
            float timerX = clockCx - CLOCK_R - GAP - bW.size.x - bW.position.x;

            tWaktu.setPosition({ timerX, 26.f });
            window.draw(tWaktu);
            drawClockIcon(window, clockCx, 38.f, CLOCK_R, timerColor);

            for (int r = 0; r < 6; r++) {
                std::vector<sf::Color> warna;
                if (r < (int)daftarTebakan.size())
                    warna = hitungWarnaBaris(daftarTebakan[r], kataRahasia);
                for (int c = 0; c < 5; c++) {
                    float x = 90.f + c * 85.f;
                    float y = 170.f + r * 85.f;
                    sf::RectangleShape box({ 70.f, 70.f });
                    box.setPosition({ x, y });
                    box.setOutlineThickness(2.f);
                    if (r < (int)daftarTebakan.size()) {
                        box.setFillColor(warna[c]);
                        box.setOutlineColor(warna[c]);
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
                        t.setPosition({ x + 35.f - b.size.x / 2.f - b.position.x,
                                        y + 35.f - b.size.y / 2.f - b.position.y });
                        window.draw(t);
                    }
                }
            }
        }

        // --------------------------------------------------------------------
        // RESULT
        // --------------------------------------------------------------------
        else if (currentState == GameState::RESULT) {
            // 1. Gambar result (you win / game over)
            if (menang) window.draw(spriteYouWin);
            else        window.draw(spriteGameOver);

            // 2. Panel statistik
            sf::ConvexShape panel = buatKotakMelengkung(520.f, RESULT_PANEL_H, 18.f, sf::Color(22, 22, 25, 235));
            panel.setPosition({ 40.f, RESULT_PANEL_Y });
            window.draw(panel);

            // Nama user (emas)
            sf::Text tNama(font, namaUser, 27);
            tNama.setFillColor(sf::Color(220, 195, 110));
            tNama.setStyle(sf::Text::Bold);
            drawCenteredText(window, tNama, RESULT_PANEL_Y + 14.f);

            // Garis pemisah
            sf::RectangleShape garis({ 400.f, 1.f });
            garis.setFillColor(sf::Color(80, 80, 80));
            garis.setPosition({ 100.f, RESULT_PANEL_Y + 55.f });
            window.draw(garis);

            // ---- Baris waktu: ikon jam + teks, di-CENTER ----
            float rowY1 = RESULT_PANEL_Y + 66.f;
            std::string waktuStr = "Waktu: " + formatWaktu(waktuTerpakai);
            sf::Text tWaktu(font, waktuStr, 22);
            tWaktu.setFillColor(sf::Color(200, 200, 200));
            sf::FloatRect bWaktu = tWaktu.getLocalBounds();

            const float ICON_R_RES = 11.f;
            const float ICON_GAP = 6.f;
            float totalW = ICON_R_RES * 2.f + ICON_GAP + bWaktu.size.x;
            float startX = 300.f - totalW / 2.f;
            float iconCx = startX + ICON_R_RES;
            float iconCy = rowY1 + ICON_R_RES + 1.f;
            drawClockIcon(window, iconCx, iconCy, ICON_R_RES, sf::Color(160, 160, 160));
            tWaktu.setPosition({ startX + ICON_R_RES * 2.f + ICON_GAP - bWaktu.position.x, rowY1 });
            window.draw(tWaktu);

            // Mode
            std::string modeStr = options.isHardMode ? "HARD MODE" : "NORMAL MODE";
            sf::Color   modeCol = options.isHardMode ? sf::Color(200, 80, 80) : sf::Color(100, 180, 100);
            sf::Text tMode(font, modeStr, 22);
            tMode.setFillColor(modeCol);
            tMode.setStyle(sf::Text::Bold);
            drawCenteredText(window, tMode, RESULT_PANEL_Y + 100.f);

            // Kata rahasia
            sf::Text tKata(font, "Kata: " + kataRahasia, 22);
            tKata.setFillColor(sf::Color(200, 200, 200));
            drawCenteredText(window, tKata, RESULT_PANEL_Y + 132.f);

            // Jumlah tebakan
            sf::Text tTebak(font, "Tebakan ke-" + std::to_string(daftarTebakan.size()) + " dari 6", 22);
            tTebak.setFillColor(sf::Color(200, 200, 200));
            drawCenteredText(window, tTebak, RESULT_PANEL_Y + 162.f);

            // 3. Tombol MAIN LAGI
            sf::ConvexShape btnPA = buatKotakMelengkung(RESULT_BTN_W, RESULT_BTN_H, 15.f, sf::Color(65, 67, 70));
            btnPA.setPosition({ RESULT_BTN_X, RESULT_BTN_Y1 });
            if (rectBtnPlayAgain.contains(mousePos)) btnPA.setFillColor(sf::Color(95, 97, 100));
            window.draw(btnPA);
            sf::Text tPA(font, "MAIN LAGI", 24);
            tPA.setFillColor(sf::Color(230, 230, 230));
            tPA.setStyle(sf::Text::Bold);
            sf::FloatRect bPA = tPA.getLocalBounds();
            tPA.setPosition({ 300.f - bPA.size.x / 2.f - bPA.position.x,
                              RESULT_BTN_Y1 + RESULT_BTN_H / 2.f - bPA.size.y / 2.f - bPA.position.y });
            window.draw(tPA);

            // 4. Tombol KEMBALI KE MENU
            sf::ConvexShape btnBM = buatKotakMelengkung(RESULT_BTN_W, RESULT_BTN_H, 15.f, sf::Color(100, 12, 24));
            btnBM.setPosition({ RESULT_BTN_X, RESULT_BTN_Y2 });
            if (rectBtnBackMenu.contains(mousePos)) btnBM.setFillColor(sf::Color(130, 15, 30));
            window.draw(btnBM);
            sf::Text tBM(font, "KEMBALI KE MENU", 22);
            tBM.setFillColor(sf::Color(230, 230, 230));
            tBM.setStyle(sf::Text::Bold);
            sf::FloatRect bBM = tBM.getLocalBounds();
            tBM.setPosition({ 300.f - bBM.size.x / 2.f - bBM.position.x,
                              RESULT_BTN_Y2 + RESULT_BTN_H / 2.f - bBM.size.y / 2.f - bBM.position.y });
            window.draw(tBM);
        }

        window.display();
    }
    return 0;
}