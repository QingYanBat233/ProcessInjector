using System.Diagnostics;
using System.Runtime.InteropServices;

class Program
{
    [DllImport("ProcessInjector.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    public static extern bool InjectDLL(uint targetProcessID, string dllPath);

    private const string CONFIG_PATH = "config";

    static void Main(string[] args)
    {
        if (!File.Exists(CONFIG_PATH)) 
        {
            Environment.Exit(1);
        }
        if (File.ReadAllLines(CONFIG_PATH).Length != 2)
        {
            Environment.Exit(2);
        }
        Process[] processes = Process.GetProcessesByName(File.ReadAllLines(CONFIG_PATH)[0]);
        if (processes.Length == 0)
        {
            Console.WriteLine("未找到目标进程！");
            return;
        }

        uint targetProcessID = (uint)processes[0].Id;
        string dllPath = File.ReadAllLines(CONFIG_PATH)[1];

        // 调用InjectDLL函数
        bool result = InjectDLL(targetProcessID, dllPath);

        if (result)
        {
            Console.WriteLine("DLL注入成功！");
        }
        else
        {
            Console.WriteLine("DLL注入失败！");
        }
    }
}