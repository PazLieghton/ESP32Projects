function love.load()
    -- Player settings
    player = {
        x = 400,
        y = 300,
        width = 50,
        height = 50,
        speed = 200,
        color = {0, 0, 1} -- Blue
    }
    
    -- Dot settings
    dots = {}
    dotSpawnTimer = 0
    dotSpawnInterval = 1.5
    
    -- Game settings
    score = 0
    gameOver = false
    font = love.graphics.newFont(24)
    
    -- Background
    background = {}
    for i = 1, 50 do
        table.insert(background, {
            x = math.random(0, love.graphics.getWidth()),
            y = math.random(0, love.graphics.getHeight()),
            size = math.random(2, 6),
            speed = math.random(10, 50)
        })
    end
    
    -- Power-up system
    powerUpActive = false
    powerUpTimer = 0
    powerUpDuration = 3.0
    powerUpCooldown = 0
    powerUpCooldownTime = 5.0
    
    -- HUD settings
    hudHeight = 50
end

function love.update(dt)
    if gameOver then return end
    
    -- Update background stars
    for _, star in ipairs(background) do
        star.y = star.y + star.speed * dt
        if star.y > love.graphics.getHeight() then
            star.y = -10
            star.x = math.random(0, love.graphics.getWidth())
        end
    end
    
    -- Player movement
    local moveX, moveY = 0, 0
    if love.keyboard.isDown("w", "up") then
        moveY = moveY - 1
    end
    if love.keyboard.isDown("s", "down") then
        moveY = moveY + 1
    end
    if love.keyboard.isDown("a", "left") then
        moveX = moveX - 1
    end
    if love.keyboard.isDown("d", "right") then
        moveX = moveX + 1
    end
    
    -- Normalize diagonal movement
    if moveX ~= 0 and moveY ~= 0 then
        moveX = moveX * 0.7071
        moveY = moveY * 0.7071
    end
    
    -- Apply power-up speed boost
    local currentSpeed = player.speed
    if powerUpActive then
        currentSpeed = player.speed * 2
    end
    
    player.x = player.x + moveX * currentSpeed * dt
    player.y = player.y + moveY * currentSpeed * dt
    
    -- Keep player on screen
    player.x = math.max(0, math.min(love.graphics.getWidth() - player.width, player.x))
    player.y = math.max(hudHeight, math.min(love.graphics.getHeight() - player.height, player.y))
    
    -- Handle power-up timer
    if powerUpActive then
        powerUpTimer = powerUpTimer - dt
        if powerUpTimer <= 0 then
            powerUpActive = false
        end
    else
        powerUpCooldown = powerUpCooldown - dt
    end
    
    -- Spawn dots
    dotSpawnTimer = dotSpawnTimer + dt
    if dotSpawnTimer > dotSpawnInterval then
        local size = math.random(8, 25)
        table.insert(dots, {
            x = math.random(0, love.graphics.getWidth() - size),
            y = -size,
            radius = size/2,
            speed = math.random(80, 150) + score * 2
        })
        dotSpawnTimer = 0
        -- Gradually increase difficulty
        dotSpawnInterval = math.max(0.3, dotSpawnInterval - 0.005)
    end
    
    -- Update dots and check collisions
    for i = #dots, 1, -1 do
        local dot = dots[i]
        dot.y = dot.y + dot.speed * dt
        
        -- Remove dots that go off screen
        if dot.y > love.graphics.getHeight() then
            table.remove(dots, i)
            score = score + 1
        end
        
        -- Collision detection
        local dx = player.x + player.width/2 - (dot.x + dot.radius)
        local dy = player.y + player.height/2 - (dot.y + dot.radius)
        local distance = math.sqrt(dx*dx + dy*dy)
        
        if distance < player.width/2 + dot.radius then
            gameOver = true
        end
    end
    
    -- Power-up activation (spacebar)
    if love.keyboard.wasPressed("space") and not powerUpActive and powerUpCooldown <= 0 then
        powerUpActive = true
        powerUpTimer = powerUpDuration
    end
end

function love.draw()
    -- Draw background stars
    love.graphics.setColor(0.2, 0.2, 0.2)
    for _, star in ipairs(background) do
        love.graphics.circle("fill", star.x, star.y, star.size)
    end
    
    -- Draw player
    if powerUpActive then
        love.graphics.setColor(0, 1, 1) -- Cyan when active
    else
        love.graphics.setColor(player.color)
    end
    love.graphics.rectangle("fill", player.x, player.y, player.width, player.height)
    
    -- Draw dots
    for _, dot in ipairs(dots) do
        love.graphics.setColor(1, 0, 0) -- Red
        love.graphics.circle("fill", dot.x + dot.radius, dot.y + dot.radius, dot.radius)
    end
    
    -- Draw HUD
    love.graphics.setColor(0.1, 0.1, 0.1, 0.7)
    love.graphics.rectangle("fill", 0, 0, love.graphics.getWidth(), hudHeight)
    
    -- Draw score
    love.graphics.setColor(1, 1, 1)
    love.graphics.setFont(font)
    love.graphics.print("Score: " .. score, 20, 15)
    
    -- Draw power-up status
    if powerUpActive then
        love.graphics.setColor(0, 1, 1) -- Cyan
        love.graphics.print("POWER-UP ACTIVE!", love.graphics.getWidth() - 200, 15)
    else
        if powerUpCooldown > 0 then
            love.graphics.setColor(0.7, 0.7, 0.7)
            love.graphics.print("COOLDOWN: " .. math.ceil(powerUpCooldown), love.graphics.getWidth() - 200, 15)
        else
            love.graphics.setColor(0, 1, 0) -- Green
            love.graphics.print("PRESS SPACE FOR POWER-UP", love.graphics.getWidth() - 250, 15)
        end
    end
    
    -- Draw power-up timer bar
    if powerUpActive then
        local barWidth = 150
        local barHeight = 10
        local progress = powerUpTimer / powerUpDuration
        love.graphics.setColor(0.3, 0.3, 0.3)
        love.graphics.rectangle("fill", 20, 40, barWidth, barHeight)
        love.graphics.setColor(0, 1, 1) -- Cyan
        love.graphics.rectangle("fill", 20, 40, barWidth * progress, barHeight)
    end
    
    -- Draw game over message
    if gameOver then
        love.graphics.setColor(0, 0, 0, 0.7)
        love.graphics.rectangle("fill", 0, 0, love.graphics.getWidth(), love.graphics.getHeight())
        love.graphics.setColor(1, 1, 1)
        love.graphics.printf("GAME OVER!\nFinal Score: " .. score .. "\nPress R to restart", 
                           0, love.graphics.getHeight()/2 - 50, 
                           love.graphics.getWidth(), "center")
    end
end

function love.keypressed(key)
    if key == "r" and gameOver then
        -- Reset game
        player.x = 400
        player.y = 300
        dots = {}
        score = 0
        gameOver = false
        dotSpawnInterval = 1.5
        powerUpActive = false
        powerUpTimer = 0
        powerUpCooldown = 0
    end
end

-- Add wasPressed function for key state tracking
function love.keypressed(key)
    if key == "r" and gameOver then
        -- Reset game
        player.x = 400
        player.y = 300
        dots = {}
        score = 0
        gameOver = false
        dotSpawnInterval = 1.5
        powerUpActive = false
        powerUpTimer = 0
        powerUpCooldown = 0
    end
end

-- Add key state tracking for spacebar
local keysPressed = {}

function love.keypressed(key)
    keysPressed[key] = true
end

function love.keyreleased(key)
    keysPressed[key] = false
end

function love.keyboard.wasPressed(key)
    return keysPressed[key]
end
