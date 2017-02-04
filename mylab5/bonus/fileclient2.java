import java.io.*;
import java.net.*;
import java.io.PrintWriter;

class fileclient2
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
	

	DatagramSocket clientSocket = new DatagramSocket();
	
	InetAddress IPAddress = InetAddress.getByName(args[0]);
	
	byte[] sendData = new byte[1024];
	
	byte[] receiveData = new byte[blockSize];

	String sendBack = "$" +args[2] +"$"+args[3];
	System.out.println(sendBack);

	
	sendData = sendBack.getBytes();

	DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNumber);

	clientSocket.send(sendPacket);
	
	System.out.println("Send to SERVER:"+ sendBack);

  	PrintWriter writer = new PrintWriter(args[3], "UTF-8");
	
  	//while(true){

	DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
	
	clientSocket.receive(receivePacket);
	
	String receiveCont = new String(receivePacket.getData());
	

	System.out.println("FROM SERVER:"+ receiveCont);
	

    writer.print(receiveCont);

    //if (receiveCont.length()  < blockSize) {
    //	break;
    	
   // }
	
//	}


    writer.close();




	clientSocket.close();
}
}