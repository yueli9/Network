import java.io.*;
import java.net.*;
import java.io.PrintWriter;

class fileclient
{
public static void main(String args[]) throws Exception
{

	if (args.length != 5) {
		System.out.println("ERROR:% fileclient hostname portnumber secretkey filename configfile.dat");
		System.exit(0);
	}
	
	int portNumber = Integer.parseInt(args[1]);

	System.out.println(portNumber);

	FileInputStream fis = new FileInputStream(args[4]);		// read configfile.dat
 
	BufferedReader br = new BufferedReader(new InputStreamReader(fis));
 
 	int blockSize =0;
	String line = null;
	while ((line = br.readLine()) != null) {
		System.out.println(line);
		blockSize = Integer.parseInt(line);
	}
	br.close();
	

	//DatagramSocket clientSocket = new DatagramSocket();

	
	//InetAddress IPAddress = InetAddress.getByName(args[0]);
	Socket clientSocket = new Socket(args[0], portNumber);
	//clientSocket.noTcpDelay(true);
	clientSocket.setTcpNoDelay(true);

	
	byte[] sendData = new byte[1024];
	

	String sendBack = "$" +args[2] +"$"+args[3];
	//System.out.println(sendBack);

	System.out.println("Send to SERVER:"+ sendBack);

	//sendData = sendBack.getBytes();

	DataOutputStream outToServer = new DataOutputStream(clientSocket.getOutputStream());
		//outToServer.writeBytes("hello");

  	BufferedReader inFromServer = new BufferedReader(new InputStreamReader(clientSocket.getInputStream()));

  	outToServer.writeBytes(sendBack);
  	//outToServer.writeUTF("Hello");

	//DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNumber);

	//clientSocket.send(sendPacket);
	

  	
  	PrintWriter writer = new PrintWriter(args[3], "UTF-8");
	
  	
/*
  	while(true){

	
	String receiveCont = new String(inFromServer.readLine());;

	System.out.println("FROM SERVER:"+ receiveCont);
	

    writer.print(receiveCont);

    if (receiveCont.length()  < blockSize) {
    	break;
    	
   	}

	}


    writer.close();




*/         int value=0;
long start = System.nanoTime();

	int i = 0;

             while((value = inFromServer.read()) != -1)
         {
         	i++;
            // converts int to character
            char c = (char)value;
            writer.print(c);
            // prints character
            //System.out.println(c);
         }
    writer.close();

	clientSocket.close();

// do stuff
long end = System.nanoTime();
long microseconds = (end - start) / 1000;

int tm = (int)microseconds/1000;

System.out.println("Total time spent: " + tm + "total bytes" + i + " bps: " + i/tm );

}
}