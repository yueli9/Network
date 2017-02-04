import java.io.*;
import java.net.*;

class fileserver
{
public static void main(String args[]) throws Exception
{
	if (args.length != 4) {
		System.out.println("ERROR:% fileclient hostname portnumber secretkey filename configfile.dat");
		System.exit(0);
	}
	i
	int portNumber = Integer.parseInt(args[2]);


	FileInputStream fis = new FileInputStream(args[5]);		// read configfile.dat
 
	BufferedReader br = new BufferedReader(new InputStreamReader(fis));
 
	String line = null;
	while ((line = br.readLine()) != null) {
		System.out.println(line);
		int blockSize = Integer.parseInt(line);
	}
	br.close();
	
	


	DatagramSocket serverSocket = new DatagramSocket(portNumber);
	byte[] receiveData = new byte[blockSize];
	byte[] sendData = new byte[blockSize];
	while(true)
	{
	DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
	serverSocket.receive(receivePacket);
	String sentence = new String( receivePacket.getData());
	System.out.println("RECEIVED:" + sentence);
	InetAddress IPAddress = receivePacket.getAddress();
	int port = receivePacket.getPort();
	String capitalizedSentence = sentence;
	sendData = capitalizedSentence.getBytes();
	DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, port);
	serverSocket.send(sendPacket);
	}
}
}