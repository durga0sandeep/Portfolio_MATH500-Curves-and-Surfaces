using UnityEngine;

public class GraphGrid : MonoBehaviour
{
    public GameObject ObjectHolder;
    public int X_Min, X_Max, Y_Min, Y_Max;

    public GameObject Mark;

    public GameObject GridLine_X, GridLine_Y;

    void Start()
    {
        int i = 0;

        //
        //Marks --> (X,Y)
        
        //
        //For x only onel is required so offing the grid for Project 01
        /*
        for(i = X_Min; i <= X_Max; i++)
        {
            var GObj = Instantiate(Mark);
            GObj.transform.position = new Vector3(i,0,0);
            GObj.GetComponent<CoordValue>().UpdateValue(i, 0);

            GObj.transform.parent = ObjectHolder.transform;
        }
        */

        //
        // For Project 01
        var GObj1 = Instantiate(Mark);
        GObj1.transform.position = new Vector3(6, 1, 0);
        GObj1.GetComponent<CoordValue>().UpdateValue(1, 0);

        GObj1.transform.parent = ObjectHolder.transform;
        //
        //           


        for (i = Y_Min; i <= Y_Max; i++)
        {
            var GObj = Instantiate(Mark);
            GObj.transform.position = new Vector3(0, i, 0);
            GObj.GetComponent<CoordValue>().UpdateValue(0, i);

            GObj.transform.parent = ObjectHolder.transform;
        }
        //
        //

        //
        //Grid Instantiation
        for (i = X_Min; i < X_Max; i++) {
            var GObj = Instantiate(GridLine_X);
            GObj.GetComponent<LineRenderer>().SetPosition(0, new Vector3(X_Min, i, 0));
            GObj.GetComponent<LineRenderer>().SetPosition(1, new Vector3(X_Max, i, 0));

            GObj.transform.parent = ObjectHolder.transform;
        }

        
        for (i = Y_Min; i < Y_Max; i++) {
            var GObj = Instantiate(GridLine_Y);
            GObj.GetComponent<LineRenderer>().SetPosition(0, new Vector3(i, Y_Min, 0));
            GObj.GetComponent<LineRenderer>().SetPosition(1, new Vector3(i, Y_Max, 0));

            GObj.transform.parent = ObjectHolder.transform;
        }
        //
        //
        


    }

}
