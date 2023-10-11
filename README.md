# CSClock
Chess Solitaire Clock (for Arduino)

The CS Clock is meant to be used with an electronic Chess board where you have to move your opponents pieces (although it may very well work just fine with a self-moving board ... although the Movement Check Delay may make that a challenge until later in the game where it takes longer for the opponent to move).  The CS Clock primarily for AI games (not online with live players).   The reasoning is to automate the stopping of the opponents clock (restarting yours) until you press a touch sensor (which will stop your clock).  

The goal of the CS Clock is to provide a player with a way to simulate a Tournament environment (with the exception of having to make the opponent's move).  With the CS Clock and a way to record moves (scoresheet or electronic notation device), the user can now practice making a move, stopping the clock and recording a move (in that order).  And when the opponents move, the movement sensor will recognized you make the opponents move and automatically stop the opponents clock. At which point the player should record the opponents move prior to making his own (while his clock is running) ... this is the reality of tournament play. 

General Startup:

First, understand that it takes about 30 seconds for the Movement Sensor to warm up.  This is normal and there's really nothing I can do about it.  Here is what I typcially do for starting the chess clock.

1. Power on.
2. Select 'Start as White'
3. Press the Stop Clock (touch sensor).
4. Wait for all 4 leds to come on ... which will occur after the movement sensor check delay (which indicates the Sensor is watching and will trigger ... once it is warmed up).
5. Wave your hand in front of the Motion Sensor until it triggers. 
6. Once it does trigger, repeat 3-5 a couple times.
7. This would be a good time to change the clock times and the movement sensor check delay, if needed.  See Below.
7. Press 'Reset'.
8. You should now be ready to play ... press Start as White or Start as Black ... if white, on the first move you are given 5 extra seconds (but only on the first over ... this just provides an opportunity for getting settled... but don't forget ... you can always 'pause' if you want).  If black, there is still a movement sensor delay before the sensor will check for movement.   Wait for the 4 leds to display and then make the first white move that the opponent should have already made.


Changing the Clock Time and Movement Sensor Check Delay

The Clock Time - This is how much time a game for you can take and a countdown will occur from this initial value, if it's your move.  Valid Range - 5 to 95 minutes. Default: 60 minutes.

The Movement Sensor Check Delay - This is how much time it takes before the movement sensor will start monitoring for movement.  Note: This is to give you time to note your move on a scoresheet or electronic notation device.   Valid Range - 5 to 15 seconds.  Default: 10 seconds.

The Movement Sensor doesn't monitor during your move just like the Stop Clock button isn't monitored during your opponents move. 

You can increase or decrease either of these settings, but it is dependent on whose side is active. Note: You may want to press 'Pause' before performing the changes. If the your clock is active, you can change the clock time.   If the opponents clock is active (blinking dots) ... you can change the movement sensor check delay (when you press 'Decrease' or 'Increase' the current value will be displayed).  After the values are set you would typically press 'Reset' and then start a new game ... although this is NOT necessary ... you can certainly continue playing with the new values.  Note: The movement sensor check delay value will disappear after the first return to the opponents clock (so after you make another move, and then the dots start blinking again ... the values will disappear).  A 'Reset' will also clear the display of both sides. if you want to make the new values the default values (and it doesn't matter what the current clock value says if it has counted down to less than the new value set), you can press the 'Save' button.  Upon power up, the new values will be retrieved and used.


Buttons:  Ignore what's written on board (left to right in order by #)

1 - Start as White
2 - Reset
3 - Reset to Start Values (typically would be followed by a #2 and possibly #7)
4 - Decrease Clock Time/Decrease Movement Sensor Check Delay (dependent on player color and which side's clock is running)
5 - Increase Clock Time/Increase Movement Sensor Check Delay (dependent on player color and which side's clock is running)
6 - Pause/Unpause
7 - Save Start Values (loaded on power up)
8 - Start as Black
