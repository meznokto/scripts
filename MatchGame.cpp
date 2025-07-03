#include <iostream>

#include "SimpleAudioEngine.h"
#include "MatchGame.h"
#include "GameOverScene.h"
#include "SplashScene.h"
#include "WinScene.h"

using namespace cocos2d;

USING_NS_CC;

#define TILE_SIZE 64
#define NUM_TILES 20
#define GAME_TILES 20

int MatchGame::positions[40][2] = {
		{0, 8},
		{2, 9},
		{2, 7},
		{4, 6},
		{4, 8},
		{4, 10},
		{6, 6},
		{6, 8},
		{6, 10},
		{6, 12},
		{7, 14},
		{9, 14},
		{8, 5},
		{8, 7},
		{8, 9},
		{10, 4},
		{10, 6},
		{10, 8},
		{10, 10},
		{11, 0},
		{11, 2},
		{12, 4},
		{12, 6},
		{12, 8},
		{14, 5},
		{14, 7},
		{16, 4},
		{16, 6},
		{16, 8},
		{18, 7},
		{18, 9},
		{20, 7},
		{20, 9},
		{22, 8},
		{22, 10},
		{24, 9},
		{24, 11},
		{26, 6},
		{26, 8},
		{28, 7}
	};
	
const std::string MatchGame::names[NUM_TILES] = {
	"blueberries",
	"chickadee",
	"deer",
	"finnish",
	"hiking",
	"jumping",
	"lake-michigan",
	"lighthouse",
	"mac",
	"maple",
	"moose",
	"oak",
	"pasty",
	"snow",
	"snowshoes",
	"superior",
	"tent",
	"tools",
	"trillium",
	"trout"
};

const std::string MatchGame::info[NUM_TILES] = {
	"Blueberries are perennial flowering plants with indigo-colored berries from the section Cyanococcus within the genus Vaccinium (a genus that also includes cranberries and bilberries). Species in the section Cyanococcus are the most common[1] fruits sold as blueberries and are native to North America (commercially cultivated highbush blueberries were not introduced into Europe until the 1930s).",
	"The chickadees are a group of North American birds in the tit family included in the genus Poecile. Species found in North America are referred as chickadees, while other species in the genus are called tits. They also have quick movements and are notably shy.",
	"The white-tailed deer (Odocoileus virginianus), also known as the whitetail, is a medium-sized deer native to the United States, Canada, Mexico, Central America, and South America as far south as Peru and Bolivia. It has also been introduced to New Zealand, Cuba, Jamaica, Hispaniola, Puerto Rico, Bermuda, Bahamas, Lesser Antilles, and some countries in Europe, such as Finland, the Czech Republic, and Serbia. In the Americas, it is the most widely distributed wild ungulate.",
	"The terms Finns and Finnish people (Finland-Swedish: finnar, Finnish: suomalaiset, Swedish: finnar) may refer in English to ethnic Finns, also known as Baltic Finns",
	"Hiking in Canada and the USA is the preferred term for a long, vigorous walk, usually on trails (footpaths), in the countryside",
	"Ski jumping is a sport in which skiers go down a take-off ramp, jump, and attempt to fly as far as possible. Judges award points for technique (often referred to as style points). The skis used for ski jumping are wide and long (260 to 275 centimetres (102 to 108 in)).",
	"Lake Michigan is one of the five Great Lakes of North America and the only one located entirely within the United States.",
	"A lighthouse is a tower, building, or other type of structure designed to emit light from a system of lamps and lenses and used as a navigational aid for maritime pilots at sea or on inland waterways.",
	"The Mackinac Bridge is a suspension bridge spanning the Straits of Mackinac to connect the Upper and Lower peninsulas of the U.S. state of Michigan. Opened in 1957, the 8,614-foot (2,626 m) bridge (familiarly known as \"Big Mac\" and \"Mighty Mac\") is the world's 16th-longest in total suspension and the longest suspension bridge between anchorages in the Western hemisphere.",
	"Acer is a genus of trees or shrubs commonly known as maple.",
	"The moose (North America) or Eurasian elk (Europe), Alces alces, is the largest extant species in the deer family. Moose are distinguished by the palmate antlers of the males; other members of the family have antlers with a dendritic (twig-like) configuration. Moose typically inhabit boreal and mixed deciduous forests of the Northern Hemisphere in temperate to subarctic climates.",
	"Quercus alba, the white oak, is one of the pre-eminent hardwoods of eastern North America. It is a long-lived oak of the family Fagaceae, native to eastern North America and found from southern Quebec west to eastern Minnesota and south to northern Florida and eastern Texas. Specimens have been documented to be over 450 years old.",
	"A pasty (sometimes known in the United States as a pastie or British pasty) is a baked pastry, a traditional variety of which is particularly associated with Cornwall, the westernmost county in England.",
	"Snow is precipitation in the form of flakes of crystalline water ice that falls from clouds. Since snow is composed of small ice particles, it is a granular material. It has an open and therefore soft, white, and fluffy structure, unless subjected to external pressure.",
	"A snowshoe is footwear for walking over the snow. Snowshoes work by distributing the weight of the person over a larger area so that the person's foot does not sink completely into the snow, a quality called \"flotation\".",
	"Lake Superior (French: Lac Supérieur) is the largest of the Great Lakes of North America. The lake is shared by Canada's Ontario and the United State's Minnesota to the north and west, and Wisconsin and Michigan to the south. It is generally considered the largest freshwater lake in the world by surface area. It is the world's third-largest freshwater lake by volume and the largest by volume in North America.",
	"A tent is a shelter consisting of sheets of fabric or other material draped over, attached to a frame of poles or attached to a supporting rope. While smaller tents may be free-standing or attached to the ground, large tents are usually anchored using guy ropes tied to stakes or tent pegs. First used as portable homes by nomadic peoples, tents are now more often used for recreational camping and temporary shelters.",
	"Mining is the extraction of valuable minerals or other geological materials from the earth from an orebody, lode, vein, seam, or reef, which forms the mineralized package of economic interest to the miner.",
	"Trillium (trillium, wakerobin, tri flower, birthroot, birthwort) is a genus of perennial flowering plants native to temperate regions of North America and Asia. It was formerly treated in the family Trilliaceae or trillium family, a part of the Liliales or lily order. The APG III system includes Trilliaceae in the family Melanthiaceae, where can be treated as the tribe Parideae.",
	"The rainbow trout (Oncorhynchus mykiss) is a species of salmonid native to cold-water tributaries of the Pacific Ocean in Asia and North America."
};

Scene* MatchGame::createScene()
{
    // 'scene' is an autorelease object
    auto scene = CCScene::create();
    
    // 'layer' is an autorelease object
    auto layer = MatchGame::create();

    // add layer as a child to scene
    scene->addChild(layer);

    // return the scene
    return scene;
}

// on "init" you need to initialize your instance
bool MatchGame::init()
{
    //////////////////////////////
    // 1. super init first
    if ( !CCLayer::init() )
    {
        return false;
    }
    
    this->scheduleUpdate();
    
    Size visibleSize = Director::getInstance()->getVisibleSize();
    Vec2 origin = Director::getInstance()->getVisibleOrigin();

	auto listener = EventListenerTouchOneByOne::create();
	listener->setSwallowTouches(true);
	listener->onTouchBegan = CC_CALLBACK_2(MatchGame::onTouchBegan, this);
	listener->onTouchMoved = CC_CALLBACK_2(MatchGame::onTouchMoved, this);
	listener->onTouchEnded = CC_CALLBACK_2(MatchGame::onTouchEnded, this);
	listener->onTouchCancelled = CC_CALLBACK_2(MatchGame::onTouchCancelled, this);
	
	this->getEventDispatcher()->addEventListenerWithSceneGraphPriority(listener, this);
	isTouching = false;
	touchPositionx = 0;
	touchPositiony = 0;
	clicked = 0;
	lasttile = 0;
	difficulty = UserDefault::getInstance()->getIntegerForKey("difficulty", 1);
	score = 0;
	noclicks = 0;
	guesses = 0;
	extra_tiles = UserDefault::getInstance()->getIntegerForKey("tiles", 0);
	
	if (GAME_TILES + extra_tiles > NUM_TILES)
	{
		extra_tiles = NUM_TILES - GAME_TILES;
	}
	
    // add a menu item to exit to the main screen
    auto quitItem = MenuItemImage::create(
                                           "menus/quit.png",
                                           "menus/quit.png",
                                           CC_CALLBACK_1(MatchGame::quitCallback, this));
    
	quitItem->setPosition(Vec2(origin.x + visibleSize.width - quitItem->getContentSize().width/2 ,
                                origin.y + visibleSize.height - quitItem->getContentSize().height/2));

    // create menu, it's an autorelease object
    auto menu = Menu::create(quitItem, NULL);
    menu->setPosition(Vec2::ZERO);
    this->addChild(menu, 1);

	// load the base map/background
	auto map = Sprite::create("blank.png");
	map->setAnchorPoint(Vec2(0, 0));
	map->setPosition(0, 0);
	this->addChild(map, 0);
	
	// load tile names into a vector for easy shuffling
	for (int i = 0; i < GAME_TILES + extra_tiles; i++)
	{
		loadnames.push_back(new std::string(names[i]));
	}
	
	// shuffle the tile names
	auto engine1 = std::default_random_engine{};
    engine1.seed(rand());
	std::shuffle(loadnames.begin(), loadnames.end(), engine1);
	
	// adjust the number of tiles needed depending on difficulty
	int loadnum = GAME_TILES;
	
	if (difficulty == 0)
	{
		loadnum = GAME_TILES / 2;
	}
	
	// load in images
	for (int i = 0; i < loadnum; i++)
	{
		Sprite *sprite = Sprite::create("buttons/" + names[i] + ".png");
		sprite->setOpacity(0);
		sprite->setAnchorPoint(Vec2(0,0));
		sprite->setTag(i);
		// UserData contains the info text to display when clicked
		sprite->setUserData(new __String(info[i]));
		tiles.push_back(sprite);
		this->addChild(tiles[tiles.size() - 1], 1);
		
		// load a matching blank tile
		sprite = Sprite::create("buttons/blank.png");
		sprite->setOpacity(255);
		sprite->setAnchorPoint(Vec2(0,0));
		blanks.push_back(sprite);
		this->addChild(blanks[blanks.size() - 1], 0);		
		
		sprite = Sprite::create("buttons/" + names[i] + ".png");
		sprite->setOpacity(0);
		sprite->setAnchorPoint(Vec2(0,0));
		sprite->setTag(i);
		sprite->setUserData(new __String(info[i]));
		tiles.push_back(sprite);
		this->addChild(tiles[tiles.size() - 1], 1);
		
		sprite = Sprite::create("buttons/blank.png");
		sprite->setOpacity(255);
		sprite->setAnchorPoint(Vec2(0,0));
		blanks.push_back(sprite);
		this->addChild(blanks[blanks.size() - 1], 0);	
		
		if (difficulty == 0)
		{
			sprite = Sprite::create("buttons/" + names[i] + ".png");
			sprite->setOpacity(0);
			sprite->setAnchorPoint(Vec2(0,0));
			sprite->setTag(i);
			sprite->setUserData(new __String(info[i]));
			tiles.push_back(sprite);
			this->addChild(tiles[tiles.size() - 1], 1);
			
			sprite = Sprite::create("buttons/blank.png");
			sprite->setOpacity(255);
			sprite->setAnchorPoint(Vec2(0,0));
			blanks.push_back(sprite);
			this->addChild(blanks[blanks.size() - 1], 0);	
			
			sprite = Sprite::create("buttons/" + names[i] + ".png");
			sprite->setOpacity(0);
			sprite->setAnchorPoint(Vec2(0,0));
			sprite->setTag(i);
			sprite->setUserData(new __String(info[i]));
			tiles.push_back(sprite);
			this->addChild(tiles[tiles.size() - 1], 1);
			
			sprite = Sprite::create("buttons/blank.png");
			sprite->setOpacity(255);
			sprite->setAnchorPoint(Vec2(0,0));
			blanks.push_back(sprite);
			this->addChild(blanks[blanks.size() - 1], 0);	
		}
	}
	
    // randomize the order
    shuffle();
    
    // assign positions for tiles.  Do the same for the blanks so
    // we know which ones to make disappear when matched
    for (std::vector<int>::size_type i = 0; i != tiles.size(); i++)
	{
		tiles[i]->setPosition((positions[i][0] * TILE_SIZE / 2) + 24, (positions[i][1] * TILE_SIZE / 2) + 164);
		blanks[i]->setPosition((positions[i][0] * TILE_SIZE / 2) + 24, (positions[i][1] * TILE_SIZE / 2) + 164);
	}
    
    // Create label for info about tile items
    infoLabel = Label::createWithTTF("", "fonts/FreeSans.ttf", 18, Size(600, 0));
    infoLabel->setColor(Color3B::BLACK);
	infoLabel->setPosition(Point(0, 0));
	infoLabel->setAnchorPoint(Point(0.0f, 0.0f));
    this->addChild(infoLabel, 0);
    
    // timer
    timerLabel = Label::createWithTTF("", "fonts/FreeSans.ttf", 18, Size(70, 0), TextHAlignment::CENTER);
    timerLabel->setColor(Color3B::BLACK);
    timerLabel->setPosition(Point(610, 0));
    timerLabel->setAnchorPoint(Point(0.0f, 0.0f));
    this->addChild(timerLabel, 0);
    
    // number of matches made
    scoreLabel = Label::createWithTTF(std::to_string(score) + " of " + std::to_string(tiles.size()/2), "fonts/FreeSans.ttf", 18, Size(55, 0), TextHAlignment::RIGHT);
    scoreLabel->setColor(Color3B::BLACK);
    scoreLabel->setPosition(Point(700, 0));
    scoreLabel->setAnchorPoint(Point(0.0f, 0.0f));
    this->addChild(scoreLabel, 0);
    
    // number of guesses
    guessesLabel = Label::createWithTTF(std::to_string(guesses) + " guesses", "fonts/FreeSans.ttf", 18, Size(80, 0), TextHAlignment::RIGHT);
    guessesLabel->setColor(Color3B::BLACK);
    guessesLabel->setPosition(Point(780, 0));
    guessesLabel->setAnchorPoint(Point(0.0f, 0.0f));
    this->addChild(guessesLabel, 0);
       
    // start the timer
    seconds = 0;
    this->schedule(schedule_selector(MatchGame::UpdateTimer), 1.0f, kRepeatForever, 1.0f);
    
    return true;
}


void MatchGame::quitCallback(Ref* pSender)
{
	auto scene = SplashScene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0, scene));
}

void MatchGame::GoToGameOverScene(cocos2d::Ref *pSender)
{
	auto scene = GameOverScene::createScene();
	Director::getInstance()->replaceScene(scene);
}

void MatchGame::update(float dt)
{
	// nothing for us to do.  cocos2d-x handles it all
}

bool MatchGame::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{
	isTouching = true;
	return true;
}

void MatchGame::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
	// not used
}

void MatchGame::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event)
{
	isTouching = false;
	
	// if we're not ready to accept new touches, do nothing
	if (noclicks)
	{
		return;
	}
	
	// select which sprite was touched
	touchPositionx = touch->getLocation().x;
	touchPositiony = touch->getLocation().y;
	Point touchPoint = touch->getLocation();
	
	// iterate over tiles to see if the touch was in one of them
	for (std::vector<int>::size_type i = 0; i != tiles.size(); i++)
	{
		Rect boundingBox = tiles[i]->getBoundingBox();
		
		if (boundingBox.containsPoint(touchPoint))
		{
			// tiles[i] was clicked.  Store it for later fading out.
			oldtile = i;
			
			if (blanks[i]->getDisplayedOpacity() < 255)
			{
				// tile was already clicked, do nothing
			}
			else
			{
				// fade in the tile
				auto action = FadeTo::create(.5, 255);
				tiles[i]->runAction(action);
				
				// play sound
				//String snd("sounds/" + names[tiles[i]->getTag()] + ".wav");
				//CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect(snd.getCString());
			
				if (clicked)
				{
					// this is the second tile selected, update guesses, check for a match,
					// hide both if they don't.	
					guesses++;
					guessesLabel->setString(std::to_string(guesses) + " guesses");
					
					if ((lasttile != (int)i) && (tiles[lasttile]->getTag() == tiles[i]->getTag()))
					{
						// tiles match!
						score++;
						scoreLabel->setString(std::to_string(score) + " of " + std::to_string(tiles.size()/2));
						
						fadetile1 = oldtile;
						fadetile2 = lasttile;
						
						this->scheduleOnce(schedule_selector(MatchGame::RemoveTiles), 1);
					
						// see if we've matched them all
						if (score * 2 >= tiles.size())
						{
							// We won!  stop the timer
							this->unschedule(schedule_selector(MatchGame::UpdateTimer));
							
							// store the stats.  If this is the first win, they
							// get a new tile for the next game
							// need some variable so we can tell them
							// the good news
							if (UserDefault::getInstance()->getIntegerForKey("seconds", 0) == 0)
							{
								UserDefault::getInstance()->setIntegerForKey("tiles", 1);
							}
					
							UserDefault::getInstance()->setIntegerForKey("seconds", seconds);
							UserDefault::getInstance()->setIntegerForKey("guesses", guesses);
							UserDefault::getInstance()->flush();
							
							// wait a bit before going to the winning screen
							this->scheduleOnce(schedule_selector(MatchGame::GoToWinScene), 2);
						}
					} 
					else
					{
						// tiles don't match.  noclicks prevents player
						// from clicking any tiles until the fade outs
						// are started, otherwise tiles can get stuck "on"
						noclicks = 1;
						
						if (difficulty == 2)
						{
							// hard mode - fade out without showing second tile
							auto actionFadeOut1 = FadeTo::create(.5, 0);
							auto actionFadeOut2 = FadeTo::create(.5, 0);
							tiles[oldtile]->runAction(actionFadeOut1);
							tiles[lasttile]->runAction(actionFadeOut2);
						}
						else
						{
							// normal and easy mode
							// fade out both tiles after a delay
							fadetile1 = oldtile;
							fadetile2 = lasttile;
							this->scheduleOnce(schedule_selector(MatchGame::FadeOutWithDelay), 1);
						}
					}
				
					// clear the info area
					infoLabel->setString("");
				
					// reset for the first click
					clicked = 0;
				}
				else
				{
					// this is the first tile selected.  
					// Display info about the item on the tile
					cocos2d::__String *info = (cocos2d::__String *)tiles[i]->getUserData();
					infoLabel->setString(info->getCString());
					
					// update setting to know we already have one tile flipped
					clicked = 1;
				
					// store the clicked tile, so we can fade it out later
					lasttile = i;
				}
			}
		}
	}
}

void MatchGame::onTouchCancelled(cocos2d::Touch *touch, cocos2d::Event *event)
{
	onTouchEnded(touch, event);
}

// fade out the last two selected tiles with a delay
void MatchGame::FadeOutWithDelay(float dt)
{
	auto actionFadeOut1 = FadeTo::create(.5, 0);
	auto actionFadeOut2 = FadeTo::create(.5, 0);
	tiles[fadetile1]->runAction(actionFadeOut1);
	tiles[fadetile2]->runAction(actionFadeOut2);
	noclicks = 0;
}

// fade out the tile and blank, revealing the background
void MatchGame::RemoveTiles(float dt)
{
	auto actionFadeOut1 = FadeTo::create(.5, 0);
	auto actionFadeOut2 = FadeTo::create(.5, 0);
	auto actionFadeOut3 = FadeTo::create(.5, 0);
	auto actionFadeOut4 = FadeTo::create(.5, 0);
	tiles[fadetile1]->runAction(actionFadeOut1);
	tiles[fadetile2]->runAction(actionFadeOut2);
	blanks[fadetile1]->runAction(actionFadeOut3);
	blanks[fadetile2]->runAction(actionFadeOut4);
	noclicks = 0;
}

int MatchGame::getDifficulty()
{
	return difficulty;
}

void MatchGame::setDifficulty(int diff)
{
	difficulty = diff;
}

// randomize the order of the tiles
void MatchGame::shuffle()
{
	auto engine = std::default_random_engine{};
    engine.seed(rand());
	std::shuffle(tiles.begin(), tiles.end(), engine);
}

void MatchGame::UpdateTimer(float dt)
{
	// update timerLabel
	timerLabel->setString(std::to_string(seconds) + " seconds");
	
	// add a second to the timer
	seconds++;
}

void MatchGame::GoToWinScene(float dt)
{
	auto scene = WinScene::createScene();
	Director::getInstance()->replaceScene(TransitionFade::create(1.0, scene));
}
