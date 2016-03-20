begin_cyclemsg(obj_copy)
    def_int(threadid)
    def_string(creatorname)
    def_string(title)
    def_int(needpassword)
    def_int(currentplayers)
    def_int(maxplayers)
    def_int(harden)
end_cyclemsg()

begin_cyclemsg(obj_snapshotobject)
    def_int(ObjectID)
    def_int(ClassType)
    def_int(TypeId)
    def_float(x)
    def_float(y)
    def_float(speed_x)
    def_float(speed_y)
    def_float(acceleration_x)
    def_float(acceleration_y)
    def_int(Visible)
    def_string(extend_param)
end_cyclemsg()

begin_cyclemsg(obj_object)
    def_int(ObjectId)
    def_int(ClassType)
    def_int(TypeId)
    def_string(wsObjectName)
    def_string(PropertyTable)
end_cyclemsg()

begin_cyclemsg(obj_role)
    def_int(roletype)
    def_int(lvl)
    def_string(rolename)
    def_int(cratetime)
    def_int(lastlogin)
    def_int(roleid)
    def_int(scenetypeid)
    def_float(x)
    def_float(y)
    def_int(bodyfashion)
    def_int(weaponfashion)
    def_int(weaponmodel)
    def_int(cityBodyFashion)
    def_string(proplist)
    def_int(enchantId)
    def_int(enchantLvl)
    def_int(awakelvl)
end_cyclemsg()

begin_cyclemsg(obj_pvp_role)
    def_int(roleid)
    def_string(rolename)
    def_int(lvl)
    def_int(roletype)           //角色的类型，什么，男剑士，女剑士之类的
    def_int(objType)            //目标类型， 0是机器人， 1是角色
    def_int(rank)
end_cyclemsg()

begin_cyclemsg(obj_pvp_log)
    def_string(rolename)
    def_int(iswin)
    def_int(isbechallenge)
    def_int(rank)
    def_int(changetype)
end_cyclemsg()

begin_cyclemsg(obj_active)
    def_int(index)
    def_int(progress)
    def_int(state)
end_cyclemsg()

begin_cyclemsg(obj_changeobject)
    def_int(objectid)
    def_string(propertytable)
end_cyclemsg()

//////////////邮件/////////////////////
begin_cyclemsg(obj_mail)
    def_string(mailId)
    def_string(mailtitle)
    def_string(sender)
    def_int(IsReaded)
    def_string(send_time)
    def_int(hasattach)
end_cyclemsg()

//////////////场景&玩家/////////////////////
begin_cyclemsg(obj_playerview)
    def_int(playerId)
    def_int(playerType)
    def_int(sex)
    def_int(x)
    def_int(y)
    def_string(EquipList)
end_cyclemsg()

begin_cyclemsg(obj_objectview)
    def_int(objectId)
    def_int(objectType)
    def_int(classId)
    def_int(x)
    def_int(y)
    def_int(isVisual)
end_cyclemsg()

begin_cyclemsg(obj_bagItem)
    def_int(index)
    def_int(itemid)
    def_int(count)
    def_int(dura)
    def_int(star)
    def_int(bind)
    def_string(json)
end_cyclemsg()

begin_cyclemsg(obj_addordelItem)
    def_int(deloradd) //1 添加 0 删除
    def_int(index)
    def_int(itemid)
    def_int(count)
    def_int(dura)
    def_int(star)
    def_int(bind)
    def_string(json)
end_cyclemsg()

begin_cyclemsg(obj_item)
    def_int(itemid)
    def_int(count)
end_cyclemsg()


begin_cyclemsg(obj_dungeonstate)
    def_int(index)
    def_int(level)
    def_int(resettimes)
    def_int(sweepinglevel)
end_cyclemsg()

////////////////技能相关//////////////////////

//可学习的技能
begin_cyclemsg(obj_studyable_skill)
    def_int(skillID)
    def_int(nextSkillID)
end_cyclemsg()

//已装备的技能
begin_cyclemsg(obj_equiped_skill)
    def_int(skillID)
    def_int(index)
end_cyclemsg()

//////////////////玩家属性相关////////////////////////
begin_cyclemsg(obj_playerItem)
    def_int(index)
    def_int(itemid)
    def_int(itemtype)
    def_int(dura)
    def_int(star)
    def_int(bind)
    def_string(json)
end_cyclemsg()

begin_cyclemsg(obj_updateEquip)
    def_int(deloradd)
    def_int(index)
    def_int(itemid)
    def_int(itemtype)
    def_int(dura)
    def_int(star)
    def_int(bind)
    def_string(json)
end_cyclemsg()

begin_cyclemsg(obj_cyclostyleItem)
    def_int(cyclostyleId)
    def_int(itemcount)
end_cyclemsg()

begin_cyclemsg(obj_roleinfo)
    def_int(roleid)
    def_string(rolename)
    def_int(lvl)
    def_int(roletype)
    def_int(viplvl)
    def_int(honorId)
    def_string(guildname)
    def_int(guildPosition)
    def_int(weaponquality)
    def_int(bodyfashion)
    def_int(weaponfashion)
    def_int(sceneid)
    def_int(cityBodyFashion)
    def_int(enchantId)
    def_int(enchantLvl)
    def_int(awakeLvl)
end_cyclemsg()

begin_cyclemsg(obj_pet_base_info)
    def_int(petid)
    def_int(petmod)
    def_int(ownerid)
end_cyclemsg()

begin_cyclemsg(obj_roleBattleProp)
    def_int(roleid)
    def_int(maxhp)
    def_int(atk)
    def_int(def)
    def_int(hit)
    def_int(dodge)
    def_int(cri)
    def_int_arr(skills)
    def_int(weaponquality)
    def_int(bodyfashion)
    def_int(weaponfashion)
    def_int(enchantId)
    def_int(enchantLvl)
    def_int(awakeLvl)
end_cyclemsg()



begin_cyclemsg(obj_posiinfo)
    def_int(roleid)
    def_int(x)
    def_int(y)
    def_float(direction)
    def_float(speed)
end_cyclemsg()

begin_cyclemsg(obj_friendRoleinfo)
    def_int(roleid)
    def_string(rolename)
    def_int(lvl)
    def_int(roletype)
    def_int(isonline)
    def_int(intimacy)
    def_int(battleForce)
    def_int(sceneid)
    def_int(isrobot)
end_cyclemsg()


begin_cyclemsg(obj_Retinue_base_info)
def_int(retinueid)
def_int(lvl)
def_int(retinuemod)
def_int(ownerid)
end_cyclemsg()

////////////////战斗相关//////////////////////
//收到普通攻击影响的目标物体
begin_cyclemsg(obj_attackedTarget)
    def_int(sourceID)
    def_int(targetID)
    def_int(effectLife)
    def_int(hitType)
    def_int(atkIndex) //技能伤害，为-1
    //def_int(posX)
end_cyclemsg()

// added by jianghan
// 被攻击信息
begin_cyclemsg(obj_beAttacked)
def_int(sourceID)
def_int(targetID)

def_int(x)
def_int(y)
def_int(orient)
def_int(atkIndex);
end_cyclemsg()

// 被技能打击信息
begin_cyclemsg(obj_beSkilled)
def_int(sourceID)
def_int(targetID)
def_int(skill_id)
def_string(effectname)

def_int(x)
def_int(y)
def_int(orient)
end_cyclemsg()
// end add


/////////////////////////任务模块////////////////////

//任务进度
begin_cyclemsg(obj_questProgress)
    def_int(questid)
    def_int_arr(progress)
end_cyclemsg()

begin_cyclemsg(obj_constelldata)
    def_int(consid)
    def_int(step)
end_cyclemsg()

begin_cyclemsg(obj_constellTimes)
	def_int(index)
	def_int(failtimes)
end_cyclemsg()


begin_cyclemsg(obj_guild_info)
    def_int(rank)                   //排名，0开始
    def_int(guildid)
    def_string(guildname)
    def_int(level)
    def_int(membernum)
    def_float(guildfortune)
    def_string(mastername)          //会长名
end_cyclemsg()

begin_cyclemsg(obj_guild_member)
    def_int(roleid)
    def_string(rolename)
    def_int(rank)                   //排名，0开始
    def_int(viplvl)                 //viplvl
    def_int(job)                    //职业
    def_int(lvl)                    //等级
    def_int(position)               //职位
    def_int(exploit)                //功勋
    def_int(lastActive)             //0在线，否则最后活跃时间
    def_int(battleForce)            //战斗力
    def_int(roletype)               //人物类型
end_cyclemsg()

begin_cyclemsg(obj_guild_skill)
    def_int(skillid)
    def_int(level)
end_cyclemsg()

begin_cyclemsg(obj_guild_applicant)     //申请者
    def_int(roleid)
    def_string(rolename)
    def_int(applytime)                  //申请时间
    def_int(job)                        //职业
    def_int(lvl)                        //等级
    def_int(batpoint)                   //战斗力
    def_int(vip)                        //vip等级
end_cyclemsg()

begin_cyclemsg(obj_petinfo)
    def_int(petid)
    def_int(isactive)
    def_int(pettypeid)
    def_int(growth)
    def_int(lvl)
    def_int(exp)
    def_int(batk)
    def_int(bdef)
    def_int(bhp)
    def_float(bdodge)
    def_float(bhit)
    def_int_arr(skills)
    def_int(capa)
    def_int(inte)
    def_int(phy)
    def_int(stre)
    def_int(star)               //星量
    def_int(starlvl)            //星级
    def_int(stage)              //升阶
    def_int_arr(lockskill)  //被锁住的技能
    def_int(verifyCode) //校验码
    def_int(mergelvl)   //合并阶数
    def_int(mergecurrentexp) //当前经验
    def_int(extlvladd)      //等级差
    def_int(maxlvladd)  //等级上限
    def_int(growthadd)  //成长值上限

    def_int(isnew)      //是否没查看过的新宠
    def_int(swallowlock) //幻兽吞噬锁
end_cyclemsg()


begin_cyclemsg(obj_petBattleProp)
    def_int(petid)
    def_int(lvl)
    def_int(petmod)
    def_int_arr(skills)
    def_int(maxHp)
    def_int(atk)
    def_int(def)
    def_int(hp)
    def_float(dodge)
    def_float(hit)
    def_float(cri)
    def_float(speed)
    def_float(incrHP) //回血
end_cyclemsg()


begin_cyclemsg(obj_retinueBattleProp)
def_int(retinueid)
def_int(lvl)
def_int(retinuemod)
def_int_arr(skills)
def_int(atk)
def_int(def)
def_float(dodge)
def_float(hit)
def_float(cri)
end_cyclemsg()


begin_cyclemsg(obj_check_pet_prop)
    def_int(petid)
    def_int(lvl)
    def_int(petmod)
    def_int(growth)
    def_int_arr(skills)
    def_int(maxHp)
    def_int(atk)
    def_int(def)
    def_int(hp)
    def_float(dodge)
    def_float(hit)
    def_int(capa)
    def_int(inte)
    def_int(phy)
    def_int(stre)
    def_float(cri)
    def_int(stage)//升阶
	def_int(mergelvl)
	def_int(maxlvladd)
end_cyclemsg()

begin_cyclemsg(obj_copyaward)
    def_int(flopid)
    def_int(findex)
    def_int_arr(dropindices)
    def_int(gold)
    def_int(exp)
    def_int(battlepoint)
    def_int(constell)
    def_int(petexp)
    def_int(enchantdust)
    def_int(enchantgold)
	def_string(awardItems)
end_cyclemsg()

begin_cyclemsg(obj_invite_code_num)
def_int(index)
def_int(count)
end_cyclemsg()

begin_cyclemsg(obj_lotteryhistory)//抽奖历史
    def_int(id)
    def_int(itemid)     //物品
    def_int(amount)     //数量
    def_int(date)       //日期
    def_int(rmb)        //消耗的金钻
end_cyclemsg()


begin_cyclemsg(obj_serverinfo)
    def_int(serverid)
    def_int(port)
    def_string(ip)
    def_int(playernum)
end_cyclemsg()

begin_cyclemsg(obj_dungeawawrd)
    def_int(secneid)
    def_int(flopindex)
    def_string(drops)
    def_int(gold)
    def_int(exp)
    def_int(battlepoint)
    def_int(constell)
    def_int(petexp)
	def_string(itemsAward)
end_cyclemsg()

begin_cyclemsg(obj_paihangdata)
    def_int(rank)
    def_string(rolename)
    def_int(roleid)
    //0表示角色，1表示机器人
    def_int(objtype)
    def_int(job)
    def_int(lvl)
    def_int(battle)
    def_int(recharge)
    def_int(consume)
    def_int(serverid)
    def_string(servername)
end_cyclemsg()

begin_cyclemsg(obj_Petpaihangdata)
    def_string(rolename)
    def_int(petid)
    def_int(modid)
    def_int(battle)
    def_int(growth)
    def_int(roleid)
    def_int(serverid)
    def_string(servername)
end_cyclemsg()

//角色对怪物造成的平均伤害---linshusen
begin_cyclemsg(obj_monster_average_hurt)
    def_int(monsterTpltID)  //怪物模板id
    def_int(averageHurt)    //角色对它造成的平均伤害值
end_cyclemsg()

//obj 对象在战斗中  当前的属性值
begin_cyclemsg(obj_current_battleprop_info)
    def_int(atk)
    def_int(hp)
    def_int(def)
    def_float(hit)
    def_float(dodge)
    def_float(cri)
end_cyclemsg()

begin_cyclemsg(obj_type_and_id)
    def_int(objType)
    def_int(objID)
end_cyclemsg()

begin_cyclemsg(obj_skill_effect_info)
def_int(skillid)
def_int(skilleffectindex)
end_cyclemsg()
// 活动状态列表
//////////////邮件/////////////////////
begin_cyclemsg(obj_actstat)
    def_int(actid)
    def_int(isopen)
end_cyclemsg()

//公会简要信息,分页发
begin_cyclemsg(obj_guild_brief_info)
    def_int(guildid)                //公会ID
    def_string(name)                //公会名
    def_int(rank)                   //排名
    def_int(level)                  //级别
    def_int(memberAmount)           //人数
    def_string(masterName)          //会长名
end_cyclemsg()

//公会申请信息
begin_cyclemsg(obj_guild_apply_info)
    def_int(roleid)
    def_int(date)                   //申请时间
    def_string(rolename)
    def_int(viplvl)
    def_int(job)
    def_int(level)
    def_int(battle)
end_cyclemsg()

//公会内部信息
begin_cyclemsg(obj_guild_internal_info)
    def_int(createdate)             //创建时间
    def_int(dismissdate)            //解散时间
    def_string(notice)              //公告
    def_float(fortune)              //财富,12点定时扣钱写日志，发回客户端这个清0
end_cyclemsg()

//公会成员信息
begin_cyclemsg(obj_guild_member_info)
    def_int(roleid)                 //角色ID
    def_int(viplvl)                 //vip等级
    def_string(name)                //角色名
    def_int(job)                    //职业
    def_int(level)                  //等级
    def_int(position)               //职位
    def_int(contribute)             //贡献
    def_int(lastactive)             //最后在线, 0表示在线
    def_int(reserveda)              //预留一
    def_int(reservedb)              //预留二
    def_int(reservedc)              //预留三
end_cyclemsg()

// 公会商店
begin_cyclemsg(obj_store_goods_info)
	def_int(index)              // 索引ID
	def_int(goodsId)            // 商品ID
	def_int(goodsNum)           // 商品数量
	def_int(type)               // 售卖类型 1金钻 2消耗
	def_int(price)              // 售卖价格
	def_int(isBuy)              // 是否购买
end_cyclemsg()

// 公会商店(占城特权)
begin_cyclemsg(obj_occupy_store_goods_info)
	def_int(index)              // 索引ID
	def_int(goodsId)            // 商品ID
	def_string(goodsName)
	def_string(goodsDesc)
	def_int(goodsNum)			// 商品数量　
	def_int(consume)			// 消耗
	def_int(occupied)			// 需占领分
	def_string(pic)				// 图片
end_cyclemsg()

//公会日志信息
begin_cyclemsg(obj_guild_log_info)
    def_int(date)                   //日期
    def_string(log)                 //日志
    def_int(level)                  //级别 eGuildLogLvl
end_cyclemsg()

// 公会聊天信息
begin_cyclemsg(obj_guild_chat)
    def_int(id)                     //编号
    def_int(date)                   //日期
    def_string(content)             //内容
    def_int(position)               //职位

    def_int(senderRoleid)                 //角色ID
    def_string(senderRoleName)            //角色名
    def_int(senderRoleType)
    def_int(senderLvl)

    def_int64(voiceid)
    def_int(voiceDuration)
    def_int(translated)                 //是否翻译成功
    def_int(messageType)                //区分语音或文字
end_cyclemsg()

begin_cyclemsg(obj_private_chat)
    def_int(date)                   //时间戳
    def_string(content)             //内容
    def_int(sendRoleid)                 //发送者roleid
    def_string(sendRoleName)                //消息发送者名字，若你发给你的好友，from为空
    def_int(sendRoleType)
    def_int(sendRoleLvl)

    def_int(receiveRoleid)          //接受者roleid
    def_string(receiveRoleName)                  //消息接受者名字，若好友发给你，to为空
    def_int(receiveRoleType)             //接收者类型
    def_int(receiveRoleLvl)
    def_int64(voiceid)
    def_int(voiceDuration)
    def_int(translated)                 //是否翻译成功
    def_int(messageType)                //区分语音或文字
end_cyclemsg()


//幻兽大比拼-参赛宠特介绍
begin_cyclemsg(obj_casino_pet)
    def_int(petid)          //宠物ID
    def_int(petmod)         //模板id
    def_string(name)        //宠物名字
    def_string(ownername)   //主人名字
    def_int(battle)         //战斗力
end_cyclemsg()

begin_cyclemsg(obj_copy_resettimes)
    def_int(copyid)
    def_int(times)
end_cyclemsg()

// 神秘商人列表
begin_cyclemsg(obj_mysteriousInfo)
    def_int(index)              // 索引ID
    def_int(goodsId)            // 商品ID
    def_int(goodsNum)           // 商品数量
    def_int(type)               // enStoreGoodsType
    def_int(price)              // 售卖价格
    def_int(isBuy)              // 是否已购买
end_cyclemsg()

// 图鉴列表
begin_cyclemsg(obj_illustrationsInfo)
    def_int(id)                 // ID
    def_int(green)              // 是否收集 绿
    def_int(blue)               // 是否收集 蓝
    def_int(purple)             // 是否收集 紫
    def_int(greenReward)        // IllustrationsState
    def_int(blueReward)         // IllustrationsState
    def_int(purpleReward)       // IllustrationsState
    def_int(allReward)          // IllustrationsState
end_cyclemsg()

// 称号列表
begin_cyclemsg(obj_honorInfo)
	def_int(id)					// ID
	def_int(readed)				// 是否new
	def_int(isLimited)			// 是否限时称号
	def_int(isExpire)			// 是否过期
	def_int(remainingTime)		// 剩余时间(秒)
end_cyclemsg()

begin_cyclemsg(obj_honorCondInfo)
	def_int(id)					// ID
	def_int(value)				// 条件值
end_cyclemsg()

begin_cyclemsg(obj_mysterious)
	def_int(indexId)			// 索引ID
	def_int(num)				// 购买次数
end_cyclemsg()


begin_cyclemsg(obj_mysterious_special_goods)
def_int(index)			// 索引ID
def_int(id)				
def_int(type)
def_int(consume)
def_string(sale)
def_int(rank)
def_string(desc)
def_int(limit_num)
end_cyclemsg()

begin_cyclemsg(obj_mysterious_exchange_goods)
def_int(index)			// 索引ID
def_int(id)				
def_int(lvlmin)
def_int(lvlmax)
def_int(exchange_id)
def_int(num)
def_int(rank)
def_string(desc)
def_int(limit_num)
end_cyclemsg()

//世界boss
begin_cyclemsg(obj_hurts)
    def_int(roleid)
    def_string(rolename)
    def_int(job)
    def_int(dmg)
end_cyclemsg()

//神秘副本
begin_cyclemsg(obj_mysticalmonster)
    def_int(monsterid)
    def_int(position_x)
    def_int(position_y)
    def_int(existtime)
end_cyclemsg()

begin_cyclemsg(obj_tlk_monster)
    def_int(monsterid)
    def_int(mod)
end_cyclemsg()

// 战斗力简要信息
begin_cyclemsg(obj_batforce_brief)
    def_int(maxhp)
    def_int(atk)
    def_int(def)
    def_int(hit)
    def_int(dodge)
    def_int(cri)
end_cyclemsg()


begin_cyclemsg(obj_fames_info)
    def_int(roleType)           //角色类型
    def_int(weaponQua)          //武器品质
    def_string(name)            //名字
    def_int(lvl)                //等级
    def_int(chantid)            //附魔id
    def_int(chantlvl)           //附魔等级
    def_int(awakeLvl)
end_cyclemsg()

begin_cyclemsg(obj_mysticalcopy)
    def_int(index)              //列表的索引
    def_int(existsec)           //剩余的秒数
    def_int(nextOpenSec)        //距离下次开放的时间
    def_int(copyType)           //类型
    def_int(enterLvl)
    def_int(isOpen)
end_cyclemsg()

begin_cyclemsg(obj_role_verify_info)
    def_int(roleid)
    def_int(verifyType)     //验证的类型(时间点) 参看枚举定义：VerifyPropType
    def_int(verifyHP)       //更改的血量--如果是刚进入副本，就是满血。如果战斗中过段，是更改的血

    def_int(maxHp)
    def_int(atk)            //攻击
    def_int(def)            //防御
    def_float(hit)          //命中
    def_float(dodge)        //闪避
    def_float(cri)          //暴击
    def_int(roleBattle)      //角色战斗力
    def_int(petBattle)       //宠物的战斗力
end_cyclemsg()




begin_cyclemsg(obj_pos_info)
    def_int(instid)
    def_int(orientation)
    def_float(posx)
    def_float(posy)
    def_float(speedx)
    def_float(speedy)
end_cyclemsg()


begin_cyclemsg(obj_worldrole_info)
    def_int(roleid)
    def_string(rolename)
    def_int(lvl)
    def_int(roletype)
    def_int(viplvl)
    def_int(honorId)
    def_string(guildname)
    def_int(guildPosition)
    def_int(weaponquality)
    def_int(bodyfashion)
    def_int(weaponfashion)
    def_int(maxhp)
    def_int(hp)
    def_int(group)
    def_int(enchantid)
    def_int(enchantlvl)
    def_int(awakelvl)
end_cyclemsg()


begin_cyclemsg(obj_worldpet_info)
    def_int(ownerid)

    def_int(petmod)
    def_int(maxhp)
    def_int(hp)
    def_int(group)
end_cyclemsg()

begin_cyclemsg(obj_worldnpc_info)
    def_int(npcmod)

    def_int(maxhp)
    def_int(hp)
    def_int(group)
end_cyclemsg()

begin_cyclemsg(obj_worldretinue_info)
    def_int(lvl)
    def_int(ownerid)
    def_int(retiunemod)
    def_int(group)
end_cyclemsg()

begin_cyclemsg(obj_worldfly_info)
    def_int(ownerid)

    def_int(skillid)
end_cyclemsg()


begin_cyclemsg(obj_realpvp_info)
    def_int(medal)                              //勋章
    def_int(honour)                             //荣誉
    def_int(winningstreak)                      //连胜
    def_int(winningtimes)                       //胜利场次
    def_int(losingtimes)                        //失败场次
    def_int(exittimes)                          //秒退次数
    def_int(recentgains)                        //最近战绩
    def_string(rolename)
    def_int(roleType)
    def_int(roleLvl)
    def_int(battleForce)
end_cyclemsg()

begin_cyclemsg(obj_realpvp_item)
    def_int(index)                              //
    def_int(itemid)                             //物品id
    def_int(consum)                             //消耗的勋章
    def_int(needHonor)                          //需要的最低荣誉值
end_cyclemsg()

begin_cyclemsg(obj_pvp_exchange_item)
    def_int(index)                              //
    def_int(itemid)                             //物品id
    def_int(consum)                             //消耗的point
end_cyclemsg()


begin_cyclemsg(obj_capsuletoy_info)
    def_int(qua)                // 扭蛋类型
    def_int(only_sell)          // 1 表示只用来卖，例如QUA4
    def_int(sell_price)         // 售卖价格
    def_int(enable_get)         // 1 表示可以免费领取
    def_int(can_everyday_free_get)  // 1表示可以每天领N次，例如qua1
    def_int(remain_time)        // 免费领取剩余时间，-1表示不存在倒计时（例如qua4那种直接卖）
    def_int(remain_free_times)  // 剩余免费领取次数，-1表示不存在每天免费次数，例如qua2,3,4
    def_int(remain_buy_times)   // 剩余购买次数,-1无限制购买次数
    def_int(is_have_special_offers)   //1 表示有下面的买X送Y活动
    def_int(special_offers_buy_count)   //买X送Y活动买的个数，没有这个活动此值为0，   例如qua4,这里为10
    def_int(special_offers_present_count)//买X送Y活动送的个数，                      例如qua4,这里为1
    def_int(buy_times)          //已经购买次数， is_have_special_offers等于1的时候才用这个参数
	def_int(award_mark)			// 买了超过10次要送一次,存1 2 3 4分别表示领取1次，2次

end_cyclemsg()

begin_cyclemsg(obj_twistegg_info)
	def_int(index)				// 扭蛋项（跟配置表项对应）
	def_int(functionSw)			// 项功能是否限时
	def_int(functionTm)			// 项功能倒计时（秒）
	def_int(isTimesLimit)		// 是否有次數限制 1有,0没有 
	def_int(freeNum)			// 剩余免费次数
	def_int(freeTm)				// 免费扭倒计时（秒）
end_cyclemsg()

begin_cyclemsg(obj_twistegg_result)
	def_int(goodsIndex)			// 物品索引
	def_int(goodsId)			// 物品id
	def_int(goodsNum)			// 物品数量
end_cyclemsg()

begin_cyclemsg(obj_friendDunge_record)
    def_int(index)              //地下城的index， 分高中初，3个等级
    def_int(record)             //该等级对应的记录， -1表示这个等级已经通关
end_cyclemsg()

begin_cyclemsg(obj_totem_info)
	def_int(totemId)	// 图腾id
	def_int(totemLvl)	// 等级 0-60
	def_int(totemNum)	// 当前等级已吞数量
end_cyclemsg()


begin_cyclemsg(obj_treasure_scene)  
    def_int(copyNum)                    //副本的序号
    def_int(sceneid)                    //服务器分配的场景id
    def_int(sceneMod)                   //场景的模板id
    def_int(maxnum)                     //人数上限
    def_int(curnum)                     //当前人数
    def_int(is_guild_member_full)       //场景中的公会长远是否已满
    def_int(copyType)                   //副本类型，是否双倍积分之类的
    def_int(isOpen)                     //是否开启
    def_string(sceneName)
end_cyclemsg()

//战斗内玩家的排行
begin_cyclemsg(obj_treasurecopy_rankdata)
    def_int(roleid)
    def_int(points)
end_cyclemsg()

begin_cyclemsg(obj_treasurecopy_guildresult)
    def_int(rank)
    def_string(guildName)
    def_int(points)
end_cyclemsg()

begin_cyclemsg(obj_treasurecopy_manor)
    def_int(index)                      //用来标记这是星期几的活动，前端根据这个转化成那个城镇的
    def_int(canGetAward)                //是否可以领取公会奖励
    def_string_arr(guildsName)          //按顺序的几个公会名字
end_cyclemsg()

begin_cyclemsg(obj_fashion_info)
    def_int(fashionid)
    def_int(isexpired)
    def_int(expiration)
end_cyclemsg()

begin_cyclemsg(obj_recharge_info)
	def_int(actIndex) // 活动唯一标识
	def_int(roleid)
	def_int(rmb)
	def_string(item)
	def_int(time)
	def_int(needRmb)
	def_int(hasGet) //0表示可以领取，1表示已经领取
end_cyclemsg()


begin_cyclemsg(obj_treasurecopy_getscore)
    def_int(instid)
    def_int(score)
end_cyclemsg()

begin_cyclemsg(obj_fighting_team_member)
    def_int(position)
    def_int(memberId)
    def_int(isCaptain)
    def_int(isReady)
    def_string(memberName)
    def_int(memberRoleType)
    def_int(weaponFasion)
    def_int(bodyfashion)
    def_int(weaponmodel)
    def_int(cityBodyFashion)
    def_int(battleForce)
    def_int(enchantId)
    def_int(enchantLvl)
end_cyclemsg()

begin_cyclemsg(obj_sync_team_info)
    def_int(teamid)
    def_int(captain)
    def_string(captainName)
    def_int(sceneMod)
    def_int(memberCount)
end_cyclemsg()

//累计充值
begin_cyclemsg(obj_accumulate_recharge)
    def_int(index)
    def_string(server_id)
    def_string(Channel)
    def_int(startdate)
    def_int(overdate)
    def_int(award_type)
    def_int(awardnum)
	def_string(configJson)
    def_string(title)
    def_string(content)
end_cyclemsg()

//累计消费
begin_cyclemsg(obj_accumulate_consume)
    def_int(index)
    def_string(server_id)
    def_string(Channel)
    def_int(startdate)
    def_int(overdate)
    def_int(award_type)
    def_int(awardnum)
	def_string(configJson)
    def_string(title)
    def_string(content)
end_cyclemsg()

//多次单笔充值
begin_cyclemsg(obj_operate_recharge)
    def_int(index)
    def_string(server_id)
    def_string(Channel)
    def_int(startdate)
    def_int(overdate)
    def_int(award_type)
    def_int(awardnum)
	def_string(configJson)
    def_string(title)
    def_string(content)
end_cyclemsg()


//单次单笔充值
begin_cyclemsg(obj_operate_once_recharge)
    def_int(index)
    def_string(server_id)
    def_string(Channel)
    def_int(startdate)
    def_int(overdate)
    def_int(award_type)
    def_int(awardnum)
	def_string(configJson)
    def_string(title)
    def_string(content)
end_cyclemsg()

//首冲奖励
begin_cyclemsg(obj_gift_activity)
    def_int(index)
    def_int(startdate)
    def_int(overdate)
    def_string(title)
    def_string(content)
end_cyclemsg()

//消耗精力换奖励
begin_cyclemsg(obj_fat_award_activity)
	def_int(index)
	def_string(server_id)
	def_string(Channel)
	def_int(startdate)
	def_int(overdate)
	def_int(award_type)
	def_int(awardnum)
	def_int(needfat1)
	def_string(configJson)
	def_string(title)
	def_string(content)
end_cyclemsg()

// 武器附魔
begin_cyclemsg(obj_enchant_info)
    def_int(enchantId)
    def_int(enchantLvl)
    def_int(currentExp)
end_cyclemsg()

//-----------------------
//时装搜集材料 info
begin_cyclemsg(obj_fashionMaterial_info)
    def_int(materialId)
    def_int(needNum)      //合成需要 数量
    def_int(hasEquipNum)  //已经自动放置 数量
end_cyclemsg()

//时装搜集信息
begin_cyclemsg(obj_fashionCollect_info)
    def_int(ObjectId)
    def_int(type)       //类型
    def_int(stage)      //阶数
    def_int_arr(materials)  //materialid
    def_int_arr(count)      //materialcount
end_cyclemsg()
//-----------------------


begin_cyclemsg(obj_dailyschedule_info)
    def_int(id)
    def_int(type)
    def_int(progress)
end_cyclemsg()


begin_cyclemsg(obj_midastouch_info)
	def_int(lastNum)	// 剩余次数
	def_int(curNum)		// 当前次数
	def_float(ration)	// 返利系数
	def_int(needrmb)	// 需要钻
	def_int(isAlchemy)	// 1能炼金，0不能炼金
	def_int(isSend)		// 1已返利，0未返利
end_cyclemsg()


begin_cyclemsg(obj_beworship_member)
    def_int(roleid)
    def_string(rolename)
    def_int(lvl)
    def_int(battle)
    def_int(job)
    def_int(beworshiptimes)
end_cyclemsg()


begin_cyclemsg(obj_cspvp_serverrank_member)
    def_int(rank)
    def_int(serverid)
    def_string(servername)
    def_int(score)
end_cyclemsg()



begin_cyclemsg(obj_cspvp_rolerank_member)
    def_int(rank)
    def_string(rolename)
    def_int(roleId)
    def_int(serverid)
    def_string(servername)
    def_int(accunum)
end_cyclemsg()


//跨服战

begin_cyclemsg(obj_crossservicewar_role)
def_int(roleid)
def_int(serverid)
def_string(rolename)
def_int(lvl)
def_int(roletype)           //角色的类型，什么，男剑士，女剑士之类的
def_int(rank)
end_cyclemsg()

begin_cyclemsg(obj_crossservicewar_log)
def_string(rolename)
def_int(iswin)
def_int(isbechallenge)
def_int(rank)
def_int(changetype)
end_cyclemsg()

/*
 *侍魂
 */

//侍魂信息
begin_cyclemsg(obj_retinue_info)
    def_int(retinueId)
    def_int(retinueMod)
    def_int(isFighting)
    def_int(lvl)
    def_int(star)
    def_int(exp)
    def_int(nextLvlExp)
    def_int(nextStarupExp)
    def_int(atk)
    def_int(def)
    def_int(hp)
    def_float(hit)
    def_float(cri)
    def_float(dodge)
    def_int(propStrength)
end_cyclemsg()

//侍魂技能
begin_cyclemsg(obj_retinue_skillInfo)
    def_int(retinueSkillId)
    def_int(skilllvl)
end_cyclemsg()

//通灵
begin_cyclemsg(obj_spiritspeak_info)
    def_int(spiritspeakId)
    def_int(petId)
    def_int_arr(retinueIds)

    def_int(atk)
    def_int(def)
    def_int(hp)
    def_float(hit)
    def_float(dodge)
    def_float(cri)
end_cyclemsg()

//跨服角色 fashion
begin_cyclemsg(obj_crossServiceRoleFashion_info)
    def_int(serverid)
    def_int(roleid)
    def_int(weaponFashion)
    def_int(bodyFashion)
    def_int(cityBodyFashion)
    def_int(weaponquality)
    def_int(index)  //客户端排行榜各自 index
    def_int(roleType)

end_cyclemsg()
