using System.Diagnostics;
using System.Runtime.InteropServices;

class Program
{
    [DllImport("ProcessInjector.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
    public static extern bool InjectDLL(uint targetProcessID, string dllPath);

    static void Main(string[] args)
    {
        Process[] processes = Process.GetProcessesByName("notepad");
        if (processes.Length == 0)
        {
            Console.WriteLine("未找到目标进程！");
            return;
        }

        uint targetProcessID = (uint)processes[0].Id;
        string dllPath = @"C:\Users\16176\source\repos\ProcessInjector\x64\Debug\ExampleDLL.dll"; // 替换为你要注入的DLL路径

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