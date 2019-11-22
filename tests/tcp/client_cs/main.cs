using System;
using System.Net;
using System.Net.Sockets;

class TCPClientCS {
	static void sendAxis(int chan, int angle) {
		byte[] buf = {
			(byte)((angle & 0x180) >> 7 | chan << 2),
			(byte)((angle & 0x7f) | 0x80),
		};

		using (
			var stream = new TcpClient("127.0.0.1", 12345).GetStream()
		) {
			stream.ReadTimeout = stream.WriteTimeout = 5000;
			stream.Write(buf, 0, buf.Length);
		}
	}

	static void Main(string[] Args) {
		if (Args.Length != 2) {
			Console.WriteLine("Usage: ./tcpclient.exe CHAN ANGLE");
			return;
		}

		sendAxis(Int32.Parse(Args[0]), Int32.Parse(Args[1]));
	}
}

