#include "Chat.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Tokenize.h"
#include "DatabaseEnv.h"
#include "Config.h"
#include "BattlegroundMgr.h"
#include "CommandScript.h"
#include "npc_1v1arena.h"

using namespace Acore::ChatCommands;

class CommandJoin1v1 : public CommandScript
{
public:
    CommandJoin1v1() : CommandScript("CommandJoin1v1") { }

    ChatCommandTable GetCommands() const override
    {
        static ChatCommandTable command1v1Table =
        {
            { "rated",       HandleQueueArena1v1Rated,         SEC_PLAYER,        Console::No },
            { "unrated",     HandleQueueArena1v1UnRated,         SEC_PLAYER,        Console::No },
        };

        static ChatCommandTable commandTable =
        {
            { "q1v1", command1v1Table },
        };

        return commandTable;
    }

    static bool HandleQueueArena1v1Rated(ChatHandler* handler, const char* args)
    {
        return HandleQueueArena1v1(handler, args, true);
    }

    static bool HandleQueueArena1v1UnRated(ChatHandler* handler, const char* args)
    {
        return HandleQueueArena1v1(handler, args, false);
    }

    static bool HandleQueueArena1v1(ChatHandler* handler, const char* /*args*/, bool isRated)
    {
        Player* player = handler->GetSession()->GetPlayer();
        if (!player)
            return false;

        if (!sConfigMgr->GetOption<bool>("Arena1v1.EnableCommand", true))
        {
            ChatHandler(player->GetSession()).SendSysMessage("加入1v1竞技场的命令已被禁用.");
            return false;
        }

        if (!sConfigMgr->GetOption<bool>("Arena1v1.Enable", true))
        {
            ChatHandler(player->GetSession()).SendSysMessage("1v1竞技场已被禁用.");
            return false;
        }

        if (player->IsInCombat())
        {
            ChatHandler(player->GetSession()).SendSysMessage("不能处于战斗状态.");
            return false;
        }

        npc_1v1arena Command1v1;

        uint32 minLevel = sConfigMgr->GetOption<uint32>("Arena1v1.MinLevel", 80);
        if (player->GetLevel() < minLevel)
        {
            ChatHandler(player->GetSession()).PSendSysMessage("你需要达到 {}+ 级以上才能参加单人竞技场.", minLevel);
            return false;
        }

        if (!player->GetArenaTeamId(sConfigMgr->GetOption<uint32>("Arena1v1.ArenaSlotID", 3)) && isRated)
        {
            // create 1v1 team if player doesn't have it
            if (!Command1v1.CreateArenateam(player, nullptr))
                return false;

            handler->PSendSysMessage("再次参加1v1评级竞技场!");
        }
        else
        {
            if (Command1v1.JoinQueueArena(player, nullptr, isRated))
                handler->PSendSysMessage("你已加入单人1v1竞技场队列 {}.", isRated ? "积分赛" : "练习赛");
        }

        return true;
    }
};

void AddSC_arena1v1_commandscript()
{
    new CommandJoin1v1();
}
