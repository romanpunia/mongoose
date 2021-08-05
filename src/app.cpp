#include <tomahawk/tomahawk.h>
#include <csignal>

using namespace Tomahawk::Core;
using namespace Tomahawk::Audio;
using namespace Tomahawk::Compute;
using namespace Tomahawk::Engine;
using namespace Tomahawk::Network;
using namespace Tomahawk::Graphics;
using namespace Tomahawk::Script;

class Runtime : public Application
{
public:
	ChangeLog* Stream = nullptr;
    Console* Log = nullptr;
    ChildProcess Process;
    std::string Filename;
    std::string Logs;
    bool Terminal;
    
public:
	explicit Runtime(Application::Desc* Conf) : Application(Conf)
	{
	}
	~Runtime() override
	{
		delete Stream;
		delete Log;
        OS::Process::Free(&Process);
    }
	void Initialize(Application::Desc* In) override
	{
        TH_INFO("loading for ./data/conf.xml");
        auto Reference = Content->Load<Document>("conf.xml");
        if (!Reference)
        {
            TH_ERR("couldn't load ./data/conf.xml (abort)");
            return Stop();
        }

        std::string N = Socket::LocalIpAddress();
        std::string D = Content->GetEnvironment();
		ProcessNode(Reference, N, D);

		NMake::Unpack(Reference->Fetch("application.terminal"), &Terminal);
        if (Terminal)
        {
            Log = Console::Get();
            Log->Show();
        }
        else
			TH_CLEAR(Log);

		Document* SystemLog = Reference->Fetch("database.systemLog.path.[v]");
		if (SystemLog != nullptr)
		{
			Logs = SystemLog->Value.Serialize();
			if (!Logs.empty())
				TH_INFO("system logs at %s", Logs.c_str());
		}

		NMake::Unpack(Reference->Fetch("application.path"), &Filename);
        if (!Filename.empty())
        {
            TH_INFO("loading MongoDB config at %s", Filename.c_str());

            auto StreamF = new FileStream();
			if (StreamF->Open(Filename.c_str(), FileMode::Binary_Write_Only))
			{
				std::string Tab; Document* Config = Reference->Find("database");
				if (Config != nullptr)
				{
					for (auto* Item : Config->GetChilds())
						ProcessYaml(Item, StreamF, Tab);
				}
			}

			delete StreamF;
            TH_INFO("MongoDB system config was saved");
        }

		std::string Path;
		NMake::Unpack(Reference->Fetch("application.executable"), &Path);
        if (!Path.empty())
        {
            std::vector<std::string> Args;
            Args.emplace_back("--config");
            Args.push_back(std::string("\"").append(Filename).append(1, '\"'));

            TH_INFO("spawning MongoDB process from %s", Path.c_str());
            if (!OS::Process::Spawn(Path, Args, &Process))
            {
                TH_ERR("MongoDB process cannot be spawned for some reason");
				delete Reference;
                return Stop();
            }
        }

		delete Reference;
		signal(SIGABRT, OnAbort);
		signal(SIGFPE, OnArithmeticError);
		signal(SIGILL, OnIllegalOperation);
		signal(SIGINT, OnCtrl);
		signal(SIGSEGV, OnInvalidAccess);
		signal(SIGTERM, OnTerminate);
#ifdef PLATFORM_UNIX
		signal(SIGPIPE, SIG_IGN);
#endif
        TH_INFO("initialization done");
		if (Terminal && !Logs.empty())
		{
            Stream = new ChangeLog(Logs);
            Stream->Process(Runtime::OnLogWrite);
		}
	}
	void Publish(Timer* Time) override
	{
		if (Log && Stream != nullptr)
            Stream->Process(Runtime::OnLogWrite);
	}

public:
	static bool CanTerminate()
	{
		static std::mutex Mutex;
		static bool Termination = false;

		Mutex.lock();
		bool Value = !Termination;
		Termination = true;
		Mutex.unlock();

		return Value;
	}
    static bool ProcessYaml(Document* Document, FileStream* Stream, std::string& Tab)
    {
        if (!Document || Document->Key.empty())
            return false;

        ::Document* Value = Document->Find("[v]");
        if (Document->IsEmpty() && !Value)
            return false;

        Stream->Write((Tab + Document->Key).c_str(), Document->Key.size() + Tab.size());
        Stream->Write(":", 1);

		if ((int)Document->Size() - (Value ? 1 : 0) > 0)
        {
            Tab.append("  ");
            Stream->Write("\n", 1);

            for (auto* Item : Document->GetChilds())
				ProcessYaml(Item, Stream, Tab);

            Tab = Tab.substr(0, Tab.size() - 2);
        }
		else if (Value)
		{
			std::string V = Value->Value.Serialize();
			Stream->Write((" " + V).c_str(), V.size() + 1);
			Stream->Write("\n", 1);
		}

        return true;
    }
	static void ProcessNode(Document* Value, const std::string& N, const std::string& D)
	{
		if (Value != nullptr)
		{
			Value->Value = Var::Auto(Parser(Value->Value.Serialize()).Path(N, D).R());
			for (auto* Item : Value->GetChilds())
				ProcessNode(Item, N, D);
		}
	}
	static void OnAbort(int Value)
	{
		signal(SIGABRT, OnAbort);
		if (CanTerminate())
			Application::Get()->Stop();
	}
	static void OnArithmeticError(int Value)
	{
		signal(SIGFPE, OnArithmeticError);
		if (CanTerminate())
			Application::Get()->Stop();
	}
	static void OnIllegalOperation(int Value)
	{
		signal(SIGILL, OnIllegalOperation);
		if (CanTerminate())
			Application::Get()->Stop();
	}
	static void OnCtrl(int Value)
	{
		signal(SIGINT, OnCtrl);
		if (CanTerminate())
			Application::Get()->Stop();
	}
	static void OnInvalidAccess(int Value)
	{
		signal(SIGSEGV, OnInvalidAccess);
		if (CanTerminate())
			Application::Get()->Stop();
	}
	static void OnTerminate(int Value)
	{
		signal(SIGTERM, OnTerminate);
		if (CanTerminate())
			Application::Get()->Stop();
	}
	static bool OnLogWrite(ChangeLog* Logger, const char* Buffer, int64_t Size)
	{
	    if (Size >= 31 && Buffer[28] == ' ')
        {
            if (Buffer[29] == 'I' || Buffer[29] == 'W' || Buffer[29] == 'E' || Buffer[29] == 'F' || Buffer[29] == 'D')
            {
	            int Offset = 30;
	            while (Buffer[Offset] == ' ')
	                Offset++;

                TH_INFO("%.*s", (size_t)Size - Offset, Buffer + Offset);
            }
            else
                TH_INFO("%.*s", (size_t)Size, Buffer);
        }
	    else
            TH_INFO("%.*s", (size_t)Size, Buffer);

		return true;
	}
};

int main()
{
    Tomahawk::Initialize((uint64_t)Tomahawk::Preset::App);
    {
        Application::Desc Interface = Application::Desc();
        Interface.Usage = (size_t)ApplicationSet::ContentSet;
		Interface.Directory = "data";
		Interface.Framerate = 6.0;
		Interface.Async = true;

        auto* App = new Runtime(&Interface);
        App->Start(&Interface);
		TH_RELEASE(App);
    }
    Tomahawk::Uninitialize();

	return 0;
}
