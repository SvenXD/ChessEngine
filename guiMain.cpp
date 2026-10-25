#include "Attacks/precomputed.h" 
#include "Board/position.h"
#include "Board/move.h"
#include "Utils/bit_utils.h"
#include <raylib.h>

struct PieceTextures {
    Texture2D wP, wN, wB, wR, wQ, wK;
    Texture2D bP, bN, bB, bR, bQ, bK;
};

// Helper function to figure out which texture belongs on a square
Texture2D get_piece_texture(const Position& pos, Square sq, const PieceTextures& tex) {
    // Your engine uses 0-11 for pieces (0=W_PAWN, 6=B_PAWN, etc.)
    for (int p = 0; p < 12; ++p) {
        if (test_bit(pos.pieces[p], sq)) {
            switch (p) {
                case 0: return tex.wP;
                case 1: return tex.wN;
                case 2: return tex.wB;
                case 3: return tex.wR;
                case 4: return tex.wQ;
                case 5: return tex.wK;
                case 6: return tex.bP;
                case 7: return tex.bN;
                case 8: return tex.bB;
                case 9: return tex.bR;
                case 10: return tex.bQ;
                case 11: return tex.bK;
            }
        }
    }
    // Return a blank/empty texture if the square is empty
    Texture2D empty = { 0 };
    return empty;
}

int main() {
    init_attack_tables();

    // --- WINDOW DIMENSIONS ---
    const int boardSize = 800;     
    const int sqSize = 100;
    const int panelWidth = 300;    
    const int screenWidth = boardSize + panelWidth; // 1100
    const int screenHeight = boardSize;             // 800

    InitWindow(screenWidth, screenHeight, "Sven_Bot GUI");
    SetTargetFPS(60);

    // --- ENGINE STATE ---
    Position pos;
    parse_fen(pos, "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // --- ASSETS ---
    Texture2D boardTex = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\blue.png)");

    PieceTextures tex;
    tex.wP = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\w_pawn_png_shadow_128px.png)");
    tex.wN = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\w_knight_png_shadow_128px.png)");
    tex.wB = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\w_bishop_png_shadow_128px.png)");
    tex.wR = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\w_rook_png_shadow_128px.png)");
    tex.wQ = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\w_queen_png_shadow_128px.png)");
    tex.wK = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\w_king_png_shadow_128px.png)");
    
    tex.bP = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\b_pawn_png_shadow_128px.png)");
    tex.bN = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\b_knight_png_shadow_128px.png)");
    tex.bB = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\b_bishop_png_shadow_128px.png)");
    tex.bR = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\b_rook_png_shadow_128px.png)");
    tex.bQ = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\b_queen_png_shadow_128px.png)");
    tex.bK = LoadTexture(R"(C:\Users\USUARIO\Documents\Code\Cpp\Chess\assets\cburnett\b_king_png_shadow_128px.png)");

    Vector2 origin = { 0.0f, 0.0f };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(Color{30, 30, 30, 255}); 

        // ==========================================
        // 1. DRAW THE BOARD
        // ==========================================
        Rectangle sourceRec = { 0.0f, 0.0f, (float)boardTex.width, (float)boardTex.height };
        Rectangle destRec = { 0.0f, 0.0f, (float)boardSize, (float)boardSize };
        DrawTexturePro(boardTex, sourceRec, destRec, origin, 0.0f, WHITE);

        // ==========================================
        // 2. DRAW THE PIECES FROM ENGINE STATE
        // ==========================================
        for (int r = 7; r >= 0; --r) {       
            for (int f = 0; f < 8; ++f) {    
                
                Square sq = make_square(f, r);
                Texture2D currentPiece = get_piece_texture(pos, sq, tex);

                // If square is empty, skip drawing
                if (currentPiece.id == 0) continue;

                int x = f * sqSize;
                int y = (7 - r) * sqSize;

                // --- Centering math for different sized assets ---
                float maxDim = (currentPiece.width > currentPiece.height) ? (float)currentPiece.width : (float)currentPiece.height;
                float scale = (float)sqSize / maxDim;
                float drawW = currentPiece.width * scale;
                float drawH = currentPiece.height * scale;
                float drawX = x + (sqSize - drawW) / 2.0f;
                float drawY = y + (sqSize - drawH) / 2.0f;

                Rectangle srcPiece = { 0.0f, 0.0f, (float)currentPiece.width, (float)currentPiece.height };
                Rectangle destPiece = { drawX, drawY, drawW, drawH };
                
                DrawTexturePro(currentPiece, srcPiece, destPiece, origin, 0.0f, WHITE);
            }
        }

        // ==========================================
        // 3. DRAW THE SIDE PANEL
        // ==========================================
        DrawRectangle(boardSize, 0, panelWidth, screenHeight, Color{45, 45, 45, 255});
        DrawText("Sven_Bot", boardSize + 20, 30, 30, RAYWHITE);
        DrawText("Ready to play", boardSize + 20, 80, 20, LIGHTGRAY);
        DrawLine(boardSize + 20, 120, screenWidth - 20, 120, GRAY);

        EndDrawing();
    }

    // Unload textures
    UnloadTexture(boardTex);
    UnloadTexture(tex.wP); UnloadTexture(tex.wN); UnloadTexture(tex.wB);
    UnloadTexture(tex.wR); UnloadTexture(tex.wQ); UnloadTexture(tex.wK);
    UnloadTexture(tex.bP); UnloadTexture(tex.bN); UnloadTexture(tex.bB);
    UnloadTexture(tex.bR); UnloadTexture(tex.bQ); UnloadTexture(tex.bK);
    
    CloseWindow();
    return 0;
}