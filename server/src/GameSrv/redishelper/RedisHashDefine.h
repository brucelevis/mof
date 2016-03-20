

// 这个东西要以后移回角色身上的，属性别放这里了。
begin_redis_hash_class(RoleGuildProp,rnRoleGuild)          //角色的公会属性
    redis_hash_int(Guild,guild)                            //公会ID
    redis_hash_obj(RoleGuildSkill,Skill,skill)             //技能
    redis_hash_int(Constrib,constrib)                      //贡献值
    redis_hash_int(Exploit,exploit)                        //功勋
    redis_hash_int(JoinDate,joindate)                      //入会时间
	redis_hash_int(Position,position)                      //职位(０表示没有职位)
end_redis_hash_class()


begin_redis_hash_class(GuildProp,rnGuild)                  //公会
    redis_hash_string(GuildName,guildname)                 //名
    redis_hash_int(CreateDate,createdate)                  //创建时间
    redis_hash_int(DissmissDate,dissmissdate)              //解散时间
    redis_hash_int(Master,master)                          //会长
    redis_hash_string(Notice,notice)                       //公告
    redis_hash_float(Fortune,fortune)                        //财富
    redis_hash_float(Construction,construction)              //建设度
    redis_hash_int(DonateRmbNum,donatermbnum)              //日捐献金钻次数
    redis_hash_int(BossExp,bossexp)                        //公会boss经验
    redis_hash_int(Inspired,inspired)                      //被鼓舞次数
    redis_hash_int(ImpeachApplicant, impeachapplicant)      //会长弹劾申请者
	redis_hash_int(TransferMasterDate, transfermasterdate)	// 转让会长的时间
	redis_hash_int(TransferMasterId, transfermasterid)	// 存储会长转让的id
end_redis_hash_class()


begin_redis_hash_class(PetNewProperty,rnPet)                //宠物新字段
    redis_hash_int(Growth,growth)                           //成长值
    redis_hash_int(Stage,stage)                             //升阶
    redis_hash_int(StarConvertedStage,starconvertedstage)   //升星转升阶了吗
    redis_hash_obj(IntArrayObj,Skill,skills)                  //技能
    redis_hash_obj(IntArrayObj,LockSkill,lockskill)           //被锁住的技能

    redis_hash_int(MergeLvl, mergelvl)                      //宠物合并升阶等级（宠物合并后的升阶等级，和上面的升阶不同）
    redis_hash_int(MergeCurrentExp, mergecurrentexp)        //宠物升阶当前经验
    redis_hash_int(MergeTotalExp, mergetotalexp)            //宠物升阶总经验
    redis_hash_int(MergeExtlvlAdd, mergeextlvladd)          //等级差增加数
    redis_hash_int(MergeMaxlvlAdd, mergemaxlvladd)          //等级上限增加数
    redis_hash_int(MergeGrowthAdd, mergegrowthadd)          //成长值增加数
    redis_hash_int(NewFlag, newflag)                        //新宠物标记
    redis_hash_obj(PetUpSkillProb, UpSkillProbData, upskillprobdata) // 幻兽技能升级补偿
    redis_hash_int(SwallowLock, swallowlock)                        //幻兽吞噬锁

end_redis_hash_class()

#pragma mark - NewRoleProperty {
begin_redis_hash_class(NewRoleProperty, rnRole)					// 角色的所有新增属性
	redis_hash_int(UseHonor, usehonor)							// 正在使用的称号
	redis_hash_int(Prestige, prestige)							// 声望
    redis_hash_obj(RoleHonorAction, HonorAction, honoraction)	// 称号离线动作
    redis_hash_int(MassFatFreshTime, massfatfreshtime)			// 精力更新时间
    redis_hash_int(LotteryFree,lotteryfree)                     // 免费抽奖次数
    redis_hash_int(LotteryRmb,lotteryrmb)                       // 金钻抽奖次数
	redis_hash_string(Activation, activation)					// 激活码离线动作
	redis_hash_int(HonorSw, honorsw)                            // 称号功能上线, 初始检查开关(1已检查)
    redis_hash_int(PetEliteTimes, petelitetimes)                // 幻兽精英本已打的次数
    redis_hash_int(PetEliteBuyTimes, petelitebuytimes)          // 幻兽精英本已购买次数
    redis_hash_obj(IntArrayObj,PetEliteFightedPet,petelitefightedpet) // 幻兽精英本今日已打的幻兽
    redis_hash_string(QueenBless, queenbless)                   //精灵女王的祝福
    redis_hash_int(IsHaveChatMsg, ishavechatmsg)             //是否读了聊天留言
    // 道具使用后增加免费使用次数（不刷新清零）
    redis_hash_int(IncrEliteCopyTimes, increlitecopytimes)          //精英副本增加次数
    redis_hash_int(IncrLotteryTimes, incrlotterytimes)              //免费抽奖增加次数
    redis_hash_int(IncrTowerdefenseTimes, incrtowerdefensetimes)    //保卫神像增加次数
    redis_hash_int(IncrFriendDungeonTimes, incrfrienddungeontimes)  // 好友地下城增加次数
    redis_hash_int(ConstellFailTimes, constellfailtimes)            //星灵点亮失败的次数
    // 上次进地下城的时间，用来判断是否是昨天进地下城，出来的时候已经是第二天了
    redis_hash_int(LastEnterDungeonTime, lastenterdungeontime)
    redis_hash_string(ListPetidOrder,listpetidorder)                   //记录幻兽顺序
    redis_hash_int(DoublePotionNum, doublePotionNum)            // 好友副本 双倍药水剩余次数

    redis_hash_int(PetCollectVer, petcollectver)                // 记录图鉴更新的版本

    redis_hash_obj(RoleTwistEgg, TwistEgg, twistegg)			// 扭蛋次数记录

	redis_hash_int(MidastouchCurNum, midastouchcurnum)			// 点石成金次数
	redis_hash_int(MidastouchIndex, midastouchindex)			// 点石成金选中项
	redis_hash_int(MidastouchIsSend, midastouchissend)			// 点石成金已返利
	redis_hash_obj(RoleMidastouchAction, MidastouchAction, midastouchisaction)	// 点石成金离线充值动作

	redis_hash_obj(RoleConstellFail, ConstellFailArr, constellfailarr)	// 星座失败

#pragma mark * 答题
    redis_hash_int(AnswerActivityTime, answeractivitytime)      // 答题:活动时间
    redis_hash_int(AnswerActivityDaySw, answeractivityDaySw)    // 答题:每天是否抽题
    redis_hash_int(AnswerCurrentNumber, answercurrentnumber)    // 答题:当前答的题目
    redis_hash_int(AnswerStartTime, answerstarttime)            // 答题:答题的开始时间
    redis_hash_int(AnswerPumpingTime, answerpumpingtime)        // 答题:抽题时间
    redis_hash_int(AnswerChangeCount, answerchangecount)        // 答题:换题次数
    redis_hash_string(AnswerOfflineItems, answerofflineitems)   // 答题:离线奖励（添加背包时会出现满的情况，则下次再补发奖励）
    redis_hash_int(AnswerLastTime, answerlasttime)              // 答题:上次答题时间
    redis_hash_int(AnswerCountNum, Answercountnum)              // 答题:答题累计次数（自然周一清0）
    redis_hash_int(AnswerCountNumReward, Answercountnumreward)
    redis_hash_int(AnswerContinueNum, Answercontinuenum)        // 答题:连续答题累计次数（自然周一清0）
    redis_hash_int(AnswerContinueNumReward, Answercontinuenumreward)
    redis_hash_int(AnswerContinueFunNum, Answercontinuefunnum)  // 答题:连续答题满分累计次数（自然周一清0）
    redis_hash_int(AnswerContinueFunNumReward, Answercontinuefunnumreward)
#pragma mark * 色子
    redis_hash_int(DiceLotteryPosition,dicelotteryposition)     // 色子抽奖当前位置
    redis_hash_int(DiceLotteryTime,dicelotterytime)             // 色子抽奖次数
#pragma mark * 公会
    redis_hash_int(GuildTrainTime,guildtraintime)               // 公会试炼次数
    redis_hash_obj(IntArrayObj,GuildBlessMate,guildblessmate)   // 祝福基友
    redis_hash_int(GuildBlessed,guildblessed)                   // 被祝福次数
    redis_hash_int(GuildDnoateNum,guildonatenum)                // 日捐献公会次数
    redis_hash_int(GuildItemTrainBossTime,guilditemtrainbosstime) // 道具培养boss次数
    redis_hash_int(GuildRmbTrainBossTime,guildrmbtrainbosstime)   // 金钻培养boss次数
    redis_hash_int(GuildInspire,guildinsprie)                     // 已鼓舞公会次数
	redis_hash_int(GoodsUpdateTime,goodsupdatetime)				// 公会商店刷新时间
	redis_hash_int(GoodsUpdateNum,goodsupdatenum)				// 公会商店刷新次数累计

    redis_hash_int(KoreaLevelAward, korealvlaward)              // 韩国版10级留言奖励是否已领
    redis_hash_int(GuildLeftTime,guildlefttime)                 // 上次公会离开时间

#pragma mark * 同步竞技场
    redis_hash_int(RealPvpMatchTimes, realpvpmatchtimes)        //同步竞技场匹配次数
    redis_hash_int(RealPvpMatchIndex, realpvpmatchindex)        //区分中午场下午场或者以后的更多场次
    redis_hash_int(RealPvpFreshMatchTimes, realpvpfreshmatchtimes) //记录上次刷新可匹配此时的时间点
    
#pragma mark * 分享奖励
	redis_hash_int(ShareAwardFirst, shareawardfirst)			// 第一次分享
	redis_hash_int(ShareAwardWeekly, shareawardweekly)			// 周分享获得奖励(自然周一清０)
	redis_hash_int(ShareAwardTime, shareawardtime)				// 获取日奖励时的时间

#pragma mark * 神秘商店
	redis_hash_int(MysteriousExchangeTm, mysteriousexchangetm)	// 记录兑换的活动时间
	redis_hash_string(MysteriousExchange, mysteriousexchange)		// 记录兑换的物品数量
	redis_hash_int(MysteriousSpecialTm, mysteriousspecialtm)	// 记录特卖的活动时间
	redis_hash_string(MysteriousSpecial, mysteriousspecial)		// 记录特卖的物品数量

//    redis_hash_int(FriendDungeFriend, frienddungefriend)        //上次好友地下城正在使用的好友

    redis_hash_int(BodyFashion, bodyfashion)
    redis_hash_int(WeaponFashion, weaponfashion)
    redis_hash_int(HomeFashion,homeFashion)

#pragma mark * 组队副本
    redis_hash_int(SyncTeamCopyAttendTimes, syncteamcopyattendtimes)
    redis_hash_int(IncrSyncTeamCopyTimesForIncome, incrsyncteamcopytimesforincome)

    //地下城最高手动通关层数
    redis_hash_int( DungMaxSweepingLevel1, dungmaxsweepinglevel1)
    redis_hash_int( DungMaxSweepingLevel2, dungmaxsweepinglevel2)
    redis_hash_int( DungMaxSweepingLevel3, dungmaxsweepinglevel3)

    //地下城扫荡开关
    redis_hash_int( DungIsSweeped1, dungissweeped1)
    redis_hash_int( DungIsSweeped2, dungissweeped2)
    redis_hash_int( DungIsSweeped3, dungissweeped3)

    //幻兽大冒险进入副本次数
    //幻兽大冒险免费次数使用记录
    redis_hash_int( PetAdventureFreeTimesUsedRecord, petadventurefreetimesusedrecord)
    //幻兽大冒险使用物品增加次数
    redis_hash_int( IncrPetAdventureTimes, incrpetadventuretimes)
    //幻兽大冒险金钻购买增加次数
    redis_hash_int( PetAdventureBuyTimes, petadventurebuytimes)
    //幻兽大冒险每天购买次数记录
    redis_hash_int( PetAdventureBuyTimesRecordPerDay, petadventurebuytimesrecordperday)

    //幻兽大冒险机器幻兽租用次数
    //幻兽大冒险机器幻兽免费租用次数使用记录
    redis_hash_int( PetAdventureRobotPetFreeRentTimesUsedRecordPerDay, petadventurerobotpetfreerenttimesusedrecordperday)
    //幻兽大冒险机器幻兽当天使用金钻购买次数记录
    redis_hash_int( PetAdventureRobotPetBuyingRentTimesRecordPerDay, petadventurerobotpetbuyingrenttimesrecordperday)

    //幻兽大冒险一般副本通关记录
    redis_hash_int( PetAdventurePassRecord, petadventurepassrecord)
    //上次进幻兽大冒险的时间，用来判断是否是昨天进幻兽大冒险，出来的时候已经是第二天了
    redis_hash_int( LastEnterPetAdventureTime, lastenterpetadventuretime)

    //幻兽大本营使用物品增加打副本百分之一百出现隐藏BOSS和哥布林的次数
    redis_hash_int( IncrMysticalCopyBOSSandGoblinTimes, incrmysticalcopybossandgoblintimes)


    redis_hash_obj(RoleDungeonState, DungeonState, roledungeonstate)	//地下城状态
    redis_hash_obj(RoleCsPvpStateData, CsPvpStateData, cspvpdata)
    redis_hash_obj(RoleWorshipData, WorshipData, worshipdata)

#pragma mark * 武器附魔
    redis_hash_int( EnchantId, enchantid)           // 当前附魔id
    redis_hash_int( EnchantLvl, enchantlvl)         // 当前附魔等级

    //收费开的背包页数
    redis_hash_int( RmbExtendedBackbagSize, rmbextendedbackbagsize)

    // 扭蛋改版，补偿开关
    redis_hash_int(EggCompensation, eggcompensation)

    //异步竞技场积分
    redis_hash_int(PvpPoints, PvpPoints)

    redis_hash_obj(RoleRmbConsumedRecord, RmbConsumedRecord, rmbconsumedrecord)
    redis_hash_obj(RoleRechargeRecord, RechargeRecord, rechargerecord)

    redis_hash_int(WorshipTimes, WorshipTimes)
    redis_hash_int(BeWorshipTimes, BeWorshipTimes)
    redis_hash_int(WorshipBuyTimes, WorshipBuyTimes)

    //装备洗练后备属性刷新时间
    redis_hash_int(EquipBackupBaptizeOverTime, EquipBackupBaptizeOverTime)

    //侍魂列表
    redis_hash_string(RetinueList, retinuelist)
    redis_hash_string(RetinueSkills, RetinueSkills)
    redis_hash_string(RetinueEquipedSkills, RetinueEquipedSkills)
    redis_hash_string(SpiritSpeak, SpiritSpeak)
    redis_hash_int(RetinueSummonCooldown, RetinueSummonCooldown)
    redis_hash_int(RetinueSummonResetTimes, RetinueSummonResetTimes)

end_redis_hash_class()
#pragma mark NewRoleProperty }

#pragma mark 实时PVP数据
begin_redis_hash_class(RealPvpData, rnRealPvpData)
    redis_hash_int(Medal, medal)                            //勋章
    redis_hash_int(Honour, honour)                          //荣誉
    redis_hash_int(WinningStreak, winningstreak)            //连胜
    redis_hash_int(WinningTimes, winningtimes)              //胜利场次
    redis_hash_int(LosingTimes, losingtimes)                //失败场次
    redis_hash_int(ExitTimes, exittimes)                    //秒退次数
    redis_hash_int(RecentGains, recentgains)                //最近战绩
    redis_hash_int(WinRate, winrate)                        //胜率
    redis_hash_int(LastFirstEnemy, lastfirstenemy)          //最后一次匹配到的对手
    redis_hash_int(LastSecondEnemy, lastsecondenemy)        //最后倒数第二次匹配到得对手
end_redis_hash_class()


begin_redis_hash_class(WardrobeData, rnWardrobeData)
    redis_hash_int(Exp, exp)
    redis_hash_int(Lvl, lvl)
    redis_hash_obj(WardrobeFashionList, WeaponFashion, weaponfashion)	//　武器
    redis_hash_obj(WardrobeFashionList, BodyFashion, bodyfashion)		//　衣服
    redis_hash_obj(WardrobeFashionList,HomeFashion,homeFashion)			//　主城
end_redis_hash_class()

#pragma mark 武器附魔
begin_redis_hash_class(EnchantDBData, rnEnchantDBData)
    redis_hash_obj(WeaponEnchantDataList, EnchantDataList, enchantDataList)
end_redis_hash_class()

begin_redis_hash_class(DailyScheduleData, rnDailySchduleData)
    redis_hash_int(DataTime, datatime)
    redis_hash_int(Point, point)
    redis_hash_obj(DailyScheduleAwardState, AwardState, awardstate)
    redis_hash_obj(DailyScheduleList, ScheduleList, schedulelist)
end_redis_hash_class()

#pragma mark 侍魂
begin_redis_hash_class(RetinueData, rnRetinue)
    redis_hash_int(ModId, modid)
    redis_hash_int(Lvl, lvl)
    redis_hash_int(Star, star)
    redis_hash_int(Exp, exp)
    redis_hash_int(StarExp, starexp)
    redis_hash_int(TolalExp, totalexp)
    redis_hash_int(Active, active)
    redis_hash_int(Owner, Owner)
end_redis_hash_class()

