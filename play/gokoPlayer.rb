load("../util/cardData.rb")
require "open3"

class GokoLogParser

  BOT_NAME = "I am BOT"
  PLAY_PROGRAM = "./a.out"
  BUY_PROGRAM = "./a.out"

  PHASE_END = -1
  PHASE_ACTION = 0
  PHASE_BUY = 1
  PHASE_CLEANUP = 2

  MAX_CARDNUM = 33
  
  FEATURE_LENGTH = 233

  def parse(rawlog, output, outputAction, drawlog)
    @player = 0

    @playerName = Array.new(2)

    @playerDeck = Array.new(2)
    @playerDeck[0] = Array.new(MAX_CARDNUM, 0)
    @playerDeck[1] = Array.new(MAX_CARDNUM, 0)
    
    @playerDiscard = Array.new(2)
    @playerDiscard[0] = Array.new(MAX_CARDNUM, 0)
    @playerDiscard[1] = Array.new(MAX_CARDNUM, 0)
    
    @playerHand = Array.new(2)
    @playerHand[0] = Array.new(MAX_CARDNUM, 0)
    @playerHand[1] = Array.new(MAX_CARDNUM, 0)

    @playerPlay = Array.new(2)
    @playerPlay[0] = Array.new(MAX_CARDNUM, 0)
    @playerPlay[1] = Array.new(MAX_CARDNUM, 0)

    @supplyCnt = Array.new(MAX_CARDNUM, 0)
    @supplyExist = Array.new(MAX_CARDNUM, 0)

    @pastFeature = Array.new(2)
    @pastFeature[0] = Array.new(0)
    @pastFeature[1] = Array.new(0)

    #generate zero Feature
    3.times{
        tempFeature = ""
        FEATURE_LENGTH.times{
            tempFeature = tempFeature + "0,"
        }
        @pastFeature[0] << tempFeature[0..-2]
        @pastFeature[1] << tempFeature[0..-2]
    }

    @lastPlay = nil
    @lastTrash = nil
    @currentPhase = PHASE_ACTION
    @currentCoin = 0
    @currentBuy = 1
    @cardData = CardData.new()
    @lastBuy = Array.new(0)
    @currentPlayer = 0

    @currentTurn = 1

    @output = output
    @outputAction = outputAction

    @reveal = Array.new(2)
    @reveal[0] = Array.new(0)
    @reveal[1] = Array.new(0)

    
    
    #add pass text to log
    log = addPass(addDrawInfo(rawlog, drawlog))

    shuffleflag = false

    log.each{|line|
      
      if(line.include?("Game Over"))
        currentPhase = PHASE_END
      end

      if(line.include?(" - cards:"))
        verifyResult(line)
      end

      if(currentPhase == PHASE_END)
        next
      end

      if(line.include?("moves deck to discards"))
        moveDeckIntoDiscards(line)
      end

      if(line[0..12] == "Supply cards:")
        parseSupply(line[13..-1])
      end

      if(line.index("starting cards:") != nil)
        if(parseStartingDeck(line) == "error")
          puts "error this is not 2 player game"
          break
        end
      end

      if(line.index("pass") !=  nil)
        cleanup(nil)
        generateGroundData(nil, @currentCoin, @currentBuy)
      end

      if(line.include?("reveals"))
        parseReveal(line)
      end

      if(line.include?("on top of deck"))
        parsePlaceTop(line)
      end

      if(line.include?("places cards in hand"))
        parsePutCardInHand(line)
      end

      if(line.include?("moves"))
        parseMoveCardInHand(line)
      end

      if(line.index("turn") != nil && line.index(": turn") != nil)
        if(line[11..line.index(":") - 1] == @playerName[1])
          @currentPlayer = 1
        elsif (line[11..line.index(":")- 1] == @playerName[0])
          @currentPlayer = 0
        elsif
          puts "error!"
        end
        @currentTurn = @currentTurn + 1
        puts("Turn#{@currentTurn / 2}")
        
        @currentCoin = 0
        @currentBuy = 1
        @lastPlay = nil
        @lastTrash = nil
        @currentPhase = PHASE_ACTION
        @lastBuy = Array.new(0)
      end

      if(line.index("plays") != nil)
        if(/\d/.match(line[line.index("-") .. -2]) != nil)
          parsePlayTreasure(line)
        else parsePlayAction(line)
        end
      end

      if(line.index("buys") != nil)
        @currentPhase = PHASE_BUY
      end
      
      if(line.index("gains") != nil)
        if(@currentPhase == PHASE_BUY) 
          parseBuy(line)
        else parseGain(line)
        end
      end

      if(line.index("trashes") != nil)
        parseTrash(line)
      end

      #書庫のダミー回避
      if(line.index("draws") != nil && !line.include?("draws and discards"))
        #detect cleanup
        if(@currentPhase == PHASE_BUY)

          @currentPhase = PHASE_CLEANUP
          
          #generate ground data here
          generateGroundData(@lastBuy, @currentCoin, @currentBuy)
          
          #execute last buy
          executeBuy()
          
          #cleanup
          cleanup(line)
          
          if(shuffleflag == true)
            reshuffle(line)
            shuffleflag = false
          end
        end
        parseDraw(line)
      end

      if(line.index("shuffles") != nil)
        puts "detect shuffle"
        if(@currentPhase == PHASE_BUY)
          shuffleflag = true
        else
          reshuffle(line)
        end
      end

      if(line.index("discards") != nil && !line.include?("draws and discards"))
        parseDiscard(line)
      end
    }

    if(@playerName[@currentPlayer] != BOT_NAME)
      return
    end
    #アクションタイミングか判定
    if(haveActionInHand() && @currentPhase == PHASE_ACTION)
      generatePlayActionData()
    end

    #購入判定タイミングか判断
    if((@lastPlay != nil && (@lastPlay.name == "Feast" || @lastPlay.name == "Remodel" || @lastPlay.name == "Workshop")) || @currentPhase == PHASE_BUY)
      #改築プレイかつ廃棄が行われていない
      if(@lastPlay != nil && @lastPlay.name == "Remodel" && !(log[-1].include?("trashes")) && @currentPhase != PHASE_BUY)
        return
      end
      generateQuestionString()
    end

    rescue => ex
      puts ex.message
  end

  def generatePlayActionData()
    
    # カード使用は敗者からも取っていい気がした
    #if(@playerName[@currentPlayer] != @winner)
    #    puts "#{@playerName[@currentPlayer]} is not #{@winner} he is loser"
    #    return
    #end
  
    feature = generateFeatureString();

    handString = ""
    for i in 0...MAX_CARDNUM do
      if(@playerHand[@currentPlayer][i] > 0)
        handString = handString + i.to_s + ","
      end
    end
    handString = handString[0...-1]

    resultString = feature + "/" + handString

    puts resultString
    @outputAction.write(resultString + "\n")

    out, err, status = Open3.capture3(PLAY_PROGRAM)
    puts out
    puts err
    puts status
  end

  def generateQuestionString()
feature = generateFeatureString();
result = @pastFeature[@currentPlayer][-3] + "," + @pastFeature[@currentPlayer][-2] + "," + @pastFeature[@currentPlayer][-1] + "," + feature + "/"
if(@currentPhase != PHASE_ACTION)
    @pastFeature[@currentPlayer] << feature
end

      @supplyCnt.each{|cnt|
          result = result + cnt.to_s + ","
      }
      
      result = result[0..-2] + "/"
      
      if(@currentPhase == PHASE_BUY)
        coin = @currentCoin
        buy = @currentBuy
      elsif(@lastPlay.name == "Feast")
        coin = 5
        buy = 1
      elsif(@lastPlay.name == "Remodel")
        coin = @lastTrash.cost + 2
        buy = 1
      elsif(@lastPlay.name == "Workshop")
        coin = 4
        buy = 1
      end
      
      result = result + coin.to_s + "/" + buy.to_s
      
      puts result
      @output.write(result + "\n")


    out, err, status = Open3.capture3(BUY_PROGRAM)
    puts out
    puts err
    puts status

  end

  def verifyResult(data)
    if(@canVerify == false)
      puts "cannot verify because of quit or resign"
      return
    end
    
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    data[data.index(" - ") + 10 .. -2].split(", ").each{|pair|
      card = @cardData.getCard(pair[pair.index(" ") + 1..-1])
      num = pair.split(" ")[0].to_i
      if(num == (@playerDeck[currentPlayer][card.num] + @playerHand[currentPlayer][card.num] + @playerDiscard[currentPlayer][card.num] + @playerPlay[currentPlayer][card.num]))
        puts "#{card.name} correct"
      else
        puts "#{card.name} incorrect i estimate #{@playerDeck[currentPlayer][card.num] + @playerHand[currentPlayer][card.num] + @playerDiscard[currentPlayer][card.num] + @playerPlay[currentPlayer][card.num]} but it was #{num}"
      end
    }
  end

  def addDrawInfo(rawlog, rdrawlog)
    log = rawlog.readlines
    drawlog = rdrawlog.readlines
    drawline = 0
    playerName = nil

    resultlog = Array.new(0)

    log.each{|line|
      if(line.include?(" - starting cards:") && playerName == nil)
        playerName = line[0..line.index(" - starting cards:") - 1]
      end
      if(playerName != nil && line.include?(playerName + " - draws ") &&  !line.include?(playerName + " - draws and discards"))
        puts line
        lineStr = line[0..line.index("draws") + 5]
        drawlog[drawline].split(":").each{|str|
          if(str.include?("."))
            puts str
            if(str[0...str.index(".")] == "throneRoom")
                lineStr = lineStr + "Throne Room" + ", "
            elsif(str[0...str.index(".")] == "councilRoom")
                lineStr = lineStr + "Council Room" + ", "
            else
                lineStr = lineStr + str[0...str.index(".")].capitalize + ", "
            end
          end
        }
        lineStr = lineStr[0..-3] + "\n"
        resultlog << lineStr
        drawline = drawline + 1
      else
        resultlog << line
      end
    }

    puts resultlog
    resultlog
  end

  def addPass(rawlog)
    lineCnt = 0
    buyflag = false
    log = rawlog
    resultlog = Array.new(0)

    sepCnt = 0

    log.each{|line|
      if(line.include?(" - resigned"))
        @canVerify = false
      end
      if(line.include?(" - quit"))
        @canVerify = false
      end
      if(line.include?("---"))
        sepCnt = sepCnt + 1
        if(sepCnt > 2)
          if(buyflag == false)
            drawCnt = 0
            rLineCnt = lineCnt - 1
            drawflag = false
            while drawCnt != 5 do
              rLineCnt = rLineCnt - 1
              puts log[rLineCnt]
              puts rLineCnt
              if(log[rLineCnt].include?("draws"))
                drawCnt = drawCnt + 1 + log[rLineCnt].count(",")
                drawflag = true
                if(drawCnt > 5)
                  rLineCnt = rLineCnt + 1
                  break
                end
              end
              if(log[rLineCnt].include?("---"))
                drawflag = false
                break
              end
              if(!log[rLineCnt].include?("draws") && !log[rLineCnt].include?("shuffles"))
                puts rLineCnt
                puts lineCnt
                puts resultlog

                rLineCnt = rLineCnt + 1
                drawflag = true
                break
              end
            end
            if(drawflag == true)
              if(resultlog[-(lineCnt - rLineCnt)-1].include?(" - shuffles"))
                resultlog[-(lineCnt - rLineCnt)-1, 0] = "pass"
              else
                resultlog[-(lineCnt - rLineCnt), 0] = "pass"
              end
            end
          end
          buyflag = false
        end
      end
      if(line.include?("buys"))
        buyflag = true
      end
      lineCnt = lineCnt + 1
      resultlog << line
    }

    puts resultlog

    resultlog
  end

  def generateGroundData(gain, coin, buy)
  feature = generateFeatureString();
  if(@currentPhase != PHASE_ACTION)
      @pastFeature[@currentPlayer] << feature
  end
    return
  
    feature = generateFeatureString();
    result = @pastFeature[@currentPlayer][-3] + "," + @pastFeature[@currentPlayer][-2] + "," + @pastFeature[@currentPlayer][-1] + "," + feature + "/"
    if(@currentPhase != PHASE_ACTION)
        @pastFeature[@currentPlayer] << feature
    end
    if(gain != nil)
      gain.each{|card|
        result = result + card.num.to_s + ","
      }
      result = result[0..-2] + "/"
    else
      result = result + "/"
    end

    @supplyCnt.each{|cnt|
      result = result + cnt.to_s + ","
    }

    result = result[0..-2] + "/" + coin.to_s + "/" + buy.to_s

    puts result
    @output.write(result + "\n")
  end

  def generateFeatureString()
    result = ""
    puts "playerdeck"
    @playerDeck[@currentPlayer].each{|cardNum|
      puts cardNum
      result = result + cardNum.to_s + ","
    }
    
    @playerHand[@currentPlayer].each{|cardNum|
      result = result + cardNum.to_s + ","
    }
    @playerDiscard[@currentPlayer].each{|cardNum|
      result = result + cardNum.to_s + ","
    }
    @playerPlay[@currentPlayer].each{|cardNum|
      result = result + cardNum.to_s + ","
    }
    
    #TODO teban
    if(@currentPlayer == 0)
      other = 1
    else other = 0
    end

    for i in 0...MAX_CARDNUM do
      result = result + (@playerDeck[other][i] + @playerHand[other][i] + @playerDiscard[other][i] + @playerPlay[other][i]).to_s + ","
    end

    for i in 0...MAX_CARDNUM do
      result = result + @supplyCnt[i].to_s + ","
    end

    for i in 0...MAX_CARDNUM do
      result = result + @supplyExist[i].to_s + ","
    end
    
    result = result + (@currentTurn / 2).to_s + ","

    result = result + @currentPlayer.to_s

    result
  end

  def executeBuy()
    @lastBuy.each{|card|
      @playerDiscard[@currentPlayer][card.num] = @playerDiscard[@currentPlayer][card.num] + 1
      @supplyCnt[card.num] = @supplyCnt[card.num] - 1

      puts "#{@playerName[@currentPlayer]} buy #{card.name}"
      
    }
  end

  def cleanup(data)
    if(data == nil)
      currentPlayer = @currentPlayer
    else
      if(data[0..data.index("-") - 2] == @playerName[0])
        currentPlayer = 0
      else currentPlayer = 1
      end
    end

    for i in 0...MAX_CARDNUM do
      @playerDiscard[currentPlayer][i] = @playerDiscard[currentPlayer][i] + @playerHand[currentPlayer][i] + @playerPlay[currentPlayer][i]
      @playerPlay[currentPlayer][i] = 0
      @playerHand[currentPlayer][i] = 0
    end

    puts "cleanup"
  end

  def moveDeckIntoDiscards(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    
    for i in 1 ... MAX_CARDNUM do
      @playerDiscard[currentPlayer][i] = @playerDeck[currentPlayer][i] + @playerDiscard[currentPlayer][i]
      @playerDeck[currentPlayer][i] = 0
    end
    
    puts "doooon"
  end

  def reshuffle(data)
  #adventurer has bug in goko
  #when we use adventurer and it causes reshuffle, the timing of reshuffle of log become strange
  
  
    if(@lastPlay != nil && @lastPlay.name == "Adventurer" && @currentPhase == PHASE_ACTION)
        puts "adventurer bug shuffle"
        return
    end
  
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
  
    if(currentPlayer == 1)
        return
    end
  
    for i in 1 ... MAX_CARDNUM do
      @playerDeck[currentPlayer][i] = @playerDiscard[currentPlayer][i] + @playerDeck[currentPlayer][i]
      @playerDiscard[currentPlayer][i] = 0
    end
    
    puts "reshuffle"
  end

  def parseMoveCardInHand(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    if(@lastPlay.name == "Library")
      currentCard = @cardData.getCard(data[data.index("moves") + 6..data.index("to hand") - 2])
      @playerHand[currentPlayer][currentCard.num] = @playerHand[currentPlayer][currentCard.num] + 1
      @playerDeck[currentPlayer][currentCard.num] = @playerDeck[currentPlayer][currentCard.num] - 1
    end
  end
  
  def parsePutCardInHand(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    if(@lastPlay.name == "Adventurer")
      data[data.index("hand:") + 7..-2].split(", ").each{|card|
        currentCard = @cardData.getCard(card)
        @playerHand[currentPlayer][currentCard.num] = @playerHand[currentPlayer][currentCard.num] + 1
        @reveal[currentPlayer] = Array.new(0)
      }
    end
  end

  def parsePlaceTop(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    if(@lastPlay.name == "Bureaucrat")
      currentCard = @cardData.getCard(data[data.index("places") + 7 .. data.index("on top of deck") - 2])
      @playerHand[currentPlayer][currentCard.num] = @playerHand[currentPlayer][currentCard.num] - 1
      @playerDeck[currentPlayer][currentCard.num] = @playerDeck[currentPlayer][currentCard.num] + 1
    end
  end

  def parseReveal(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end

    if(@lastPlay.name == "Thief")
      data[data.index("reveals") + 9..-2].split(", ").each{|card|
        currentCard = @cardData.getCard(card)
        @playerDeck[currentPlayer][currentCard.num] = @playerDeck[currentPlayer][currentCard.num] - 1
        @reveal[currentPlayer] << currentCard
      }
    elsif(@lastPlay.name == "Adventurer")
      data[data.index("reveals") + 8..-2].split(", ").each{|card|
        puts card
        currentCard = @cardData.getCard(card)
        
        if(@playerDeck[currentPlayer][currentCard.num] == 0)
            puts "actual reshuffle is here"
            
            for i in 1 ... MAX_CARDNUM do
                @playerDeck[currentPlayer][i] = @playerDiscard[currentPlayer][i]
                @playerDiscard[currentPlayer][i] = 0
            end
            
        end
        
        @playerDeck[currentPlayer][currentCard.num] = @playerDeck[currentPlayer][currentCard.num] - 1
        @reveal[currentPlayer] << currentCard
      }
    end

  end

  def parseDiscard(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    
    if(currentPlayer == 1)
        return
    end
    
    handCount = 0

    #check library
    if(@lastPlay.name == "Library")
      @playerHand[currentPlayer].each{|num|
        handCount = handCount + num
      }

      emptyflag = true
      @playerDeck[currentPlayer].each{|num|
        if(num != 0)
          emptyflag = false
          break
        end
      }
      if(emptyflag == true)
        handCount = 7
      end
    end

    data = data.delete(":")
    data[data.index("discards") + 9..-2].split(", ").each{|card|

      currentCard = @cardData.getCard(card)
      
      puts "#{@playerName[currentPlayer]} discards #{currentCard.name}"
      
      if(@lastPlay.name == "Thief")
        @reveal[currentPlayer].each{|rCard|
          if(rCard.num == currentCard.num)
            @reveal.delete(rCard)
            break
          end
        }
        @playerDiscard[currentPlayer][currentCard.num] = @playerDiscard[currentPlayer][currentCard.num] + 1
      elsif(@lastPlay.name == "Spy")
        @playerDeck[currentPlayer][currentCard.num] = @playerDeck[currentPlayer][currentCard.num] - 1
        @playerDiscard[currentPlayer][currentCard.num] = @playerDiscard[currentPlayer][currentCard.num] + 1
      elsif(@lastPlay.name == "Adventurer")
        @playerDiscard[currentPlayer][currentCard.num] = @playerDiscard[currentPlayer][currentCard.num] + 1
      elsif(@lastPlay.name == "Library")
        @playerDiscard[currentPlayer][currentCard.num] = @playerDiscard[currentPlayer][currentCard.num] + 1
        @playerDeck[currentPlayer][currentCard.num] = @playerDeck[currentPlayer][currentCard.num] - 1
      else
        @playerHand[currentPlayer][currentCard.num] = @playerHand[currentPlayer][currentCard.num] - 1
        @playerDiscard[currentPlayer][currentCard.num] = @playerDiscard[currentPlayer][currentCard.num] + 1
      end
    }
  end
  
  def parseDraw(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end

    if(currentPlayer != @player)
      return
    end

    puts data

    data[data.index("draws") + 6..-2].split(", ").each{|card|

      puts card
      currentCard = @cardData.getCard(card)
      
       puts "#{@playerName[currentPlayer]} drawes #{currentCard.name}"

      @playerDeck[currentPlayer][currentCard.num] = @playerDeck[currentPlayer][currentCard.num] - 1
      @playerHand[currentPlayer][currentCard.num] = @playerHand[currentPlayer][currentCard.num] + 1

    }
  end

  def parseTrash(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    

    data[data.index("trashes") + 8..-2].split(", ").each{|card|

      currentCard = @cardData.getCard(card)
      
      if(@lastPlay.name == "Moneylender" && currentCard.name == "Copper")
        @currentCoin = @currentCoin + 3
        puts "Moneylender generates 3coins"
      end

      puts "#{@playerName[currentPlayer]} trashes #{currentCard.name}"

      if(currentCard.name == "Feast")
        @playerPlay[currentPlayer][currentCard.num] = @playerPlay[currentPlayer][currentCard.num] - 1
      elsif(@lastPlay.name == "Thief")
        @reveal[currentPlayer].each{|rCard|
          if(rCard.num == currentCard.num)
            @reveal.delete(rCard)
            break
          end
        }
      else
        @playerHand[currentPlayer][currentCard.num] = @playerHand[currentPlayer][currentCard.num] - 1
      end

      @lastTrash = currentCard
    }
  end

  def parseBuy(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    gainCard = @cardData.getCard(data[data.index("gains") + 6 .. -2])

    @lastBuy << gainCard

    puts "#{@playerName[currentPlayer]} buys #{gainCard.name} coin is #{@currentCoin} buy is #{@currentBuy}"

    
  end

  def parseGain(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    gainCard = @cardData.getCard(data[data.index("gains") + 6 .. -2])

    if(@lastPlay.name == "Feast")
      generateGroundData(Array.new(1, gainCard), 5, 1)
    elsif(@lastPlay.name == "Remodel")
      generateGroundData(Array.new(1, gainCard), @lastTrash.cost + 2, 1)
    elsif(@lastPlay.name == "Workshop")
      generateGroundData(Array.new(1, gainCard), 4, 1)
    end

    if(@lastPlay.name == "Mine")
      @playerHand[currentPlayer][gainCard.num] = @playerHand[currentPlayer][gainCard.num] + 1
      @supplyCnt[gainCard.num] = @supplyCnt[gainCard.num] - 1
    elsif(@lastPlay.name == "Bureaucrat")
      @playerDeck[currentPlayer][gainCard.num] = @playerDeck[currentPlayer][gainCard.num] + 1
      @supplyCnt[gainCard.num] = @supplyCnt[gainCard.num] - 1
    else
      @playerDiscard[currentPlayer][gainCard.num] = @playerDiscard[currentPlayer][gainCard.num] + 1
      @supplyCnt[gainCard.num] = @supplyCnt[gainCard.num] - 1
    end
    puts "#{@playerName[currentPlayer]} gains #{gainCard.name}"
  end

  def parsePlayTreasure(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end
    
    @currentPhase = PHASE_BUY

    playList = data[data.index("plays") + 6 .. -2].split(", ")
    
    playList.each{|playCard|
      
      currentCard = @cardData.getCard(playCard[2..-1])

      puts "#{@playerName[currentPlayer]} uses #{playCard[2..-1]} num is #{playCard[0]}"
      @currentCoin = @currentCoin + currentCard.coin * playCard[0].to_i
      puts "gain #{currentCard.coin * playCard[0].to_i} coins"
      @currentBuy = @currentBuy + currentCard.buy * playCard[0].to_i
      puts "gain #{currentCard.buy * playCard[0].to_i} buy"
     
      @playerHand[currentPlayer][currentCard.num] = @playerHand[currentPlayer][currentCard.num] - 1
      @playerPlay[currentPlayer][currentCard.num] = @playerPlay[currentPlayer][currentCard.num] + 1
      
    }
  
  end

  def parsePlayAction(data)
    if(data[0..data.index("-") - 2] == @playerName[0])
      currentPlayer = 0
    else currentPlayer = 1
    end

    pCard = @cardData.getCard(data[data.index("plays") + 6 .. -2])

    
    puts "#{@playerName[currentPlayer]} uses action #{data[data.index("plays") + 6 .. -2]}"
    @currentCoin = @currentCoin + pCard.coin
    puts "gain #{pCard.coin} coins"
    @currentBuy = @currentBuy + pCard.buy
    puts "gain #{pCard.buy} buy"
    
    
    
    if(@lastPlay == nil || @lastPlay.name != "Throne Room")                              
      @playerHand[currentPlayer][pCard.num] = @playerHand[currentPlayer][pCard.num] - 1
      @playerPlay[currentPlayer][pCard.num] = @playerPlay[currentPlayer][pCard.num] + 1
    end
    
    if(@lastPlay != nil && @lastPlay.name == "Throne Room" && pCard.name == "Feast")
        puts "use Throne For Feast"
       @playerPlay[currentPlayer][pCard.num] = @playerPlay[currentPlayer][pCard.num] + 1
    end
    
    @lastPlay = pCard

    if(pCard.name == "Throne Room")
	if(haveActionInHand() == false)
	  @lastPlay = nil
      puts "uses throne but have no action"
	end
    end

  end

  def haveActionInHand()
    for i in 0...MAX_CARDNUM do
      if(@playerHand[@currentPlayer][i] > 0 && @cardData.getCardByNum(i).isAction)
        return true
      end
    end
    return false
  end

  def parseStartingDeck(data)
    if(@playerName[0] == nil) 
      plNum = 0
    elsif(@playerName[1] == nil) 
      plNum = 1
    else
      return "error"
    end
      
    @playerName[plNum] = data[0..data.index("-") - 2]

    data[data.index(":") + 2..-2].split(", ").each{|card|
      
      puts card
      
      currentCard = @cardData.getCard(card)
      
      @playerDiscard[plNum][currentCard.num] = @playerDiscard[plNum][currentCard.num] + 1
    }
  end

  def parseSupply(data)
    data[1..-2].split(", ").each{|card|
      supCard = @cardData.getCard(card)
      @supplyCnt[supCard.num] = supCard.pilenum
      @supplyExist[supCard.num] = 1
    }
  end

end
