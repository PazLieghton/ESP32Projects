import pygame
import random

# Inicializar Pygame
pygame.init()

# Configurar pantalla
WIDTH, HEIGHT = 400, 300
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("Juego de Dados 🎲")

# Colores
WHITE = (255, 255, 255)
BLACK = (0, 0, 0)
RED = (200, 0, 0)

# Fuente
font = pygame.font.SysFont(None, 50)

# Función para mostrar texto
def draw_text(text, color, y):
    img = font.render(text, True, color)
    rect = img.get_rect(center=(WIDTH // 2, y))
    screen.blit(img, rect)

# Loop principal
running = True
dice_roll = None
while running:
    screen.fill(WHITE)

    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False

        if event.type == pygame.MOUSEBUTTONDOWN:
            dice_roll = random.randint(1, 6)

    draw_text("Click para tirar el dado", BLACK, 60)

    if dice_roll:
        draw_text(f"Salió: {dice_roll}", RED, 160)

    pygame.display.flip()

pygame.quit()
