import java.io.*;
import java.net.*;
class UDPClient
{
public static void main(String args[]) throws Exception
{

	if (args.length != 6) {
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
	

	//BufferedReader inFromUser = new BufferedReader(new InputStreamReader(System.in));
	
	DatagramSocket clientSocket = new DatagramSocket();
	
	InetAddress IPAddress = InetAddress.getByName(args[1]);
	

	byte[] sendData = new byte[1024];
	
	byte[] receiveData = new byte[blockSize];
	
	String sentence = inFromUser.readLine();
	
	sendData = sentence.getBytes();
	
	DatagramPacket sendPacket = new DatagramPacket(sendData, sendData.length, IPAddress, portNumber);
	
	clientSocket.send(sendPacket);
	
	DatagramPacket receivePacket = new DatagramPacket(receiveData, receiveData.length);
	
	clientSocket.receive(receivePacket);
	
	String modifiedSentence = new String(receivePacket.getData());
	System.out.println("FROM SERVER:"+ modifiedSentence);
	clientSocket.close();
}
}
