using System.Collections.Generic;
using UnityEngine;

public class CurveSystem : MonoBehaviour
{
    public List<Vector3> ControlPoints = new List<Vector3>();
    public List<GameObject> RealGObj = new List<GameObject>();
    public LineRenderer GraphLine;

    public GameObject ContrlPointPrefab;//Prefab

    [SerializeField]
    int nPoints;//Control Points Count

    float Limit_t = 1; //"t" axis has 1 as the max value
    int Y_Value = 1;
    float GeometricVal = 6;

    [SerializeField]
    float DistBt_Pts = 0;

    [SerializeField]
    float Dist = 0f;

    float t = 0;
    int n = 0;
    float bernstein = 0;

    Vector3[] positions = new Vector3[50];
    Vector3 point;
    Vector3 value;

    //
    // No.of Curve Points to change
    int NumOfCurvePts = 30;
    
    //
    void Start( )
    {
        nPoints = 0;
        GraphLine.positionCount = NumOfCurvePts;//Setting position in the linerenderer
        DrawCurve();
    }

    //
    //Control Points Generated in this code
    public void PointsGenerated(int d)
    {
          
        if (ControlPoints.Count > 0)
        {
            Clearlist( );
        }
        
        //For Conditions
        //Min d value is 1
        //Max d value is > 20
        nPoints = d + 1;
        DistBt_Pts = Limit_t / d;
        Dist = 0;

        for (int i = 0; i < nPoints; i++)
        {
            var Gobj = Instantiate(ContrlPointPrefab); //
            Gobj.name = i.ToString();
            RealGObj.Add( Gobj );
            //
            //Graphical Positions
            Gobj.transform.position = new Vector3(Dist * GeometricVal, Y_Value, 0);//Here multiplied by 6
            Gobj.transform.rotation = Quaternion.identity;
            Gobj.GetComponent<PointData>().Index = i;
            ControlPoints.Add(Gobj.transform.position);
            Dist += DistBt_Pts;
        }

    }


    //Clear the list
    void Clearlist()
    {
        int Count = ControlPoints.Count;

        for(int i = 0; i < Count; i++)
        {
            var g = ControlPoints[0];
            Destroy(RealGObj[0]);

            ControlPoints.RemoveAt(0);
            RealGObj.RemoveAt(0);
        }

        ControlPoints.Clear();
    }

    //
    //Update Method
    private void Update()
    {
        DrawCurve();
    }


 
    void DrawCurve( )
    {
        for (int i = 0; i < NumOfCurvePts; i++)
        {
            t = i / (float)(NumOfCurvePts - 1);
            if (SingletonScript.Instance.OptionChange == 0)
            {//BB Form
                point = BB_Form(t, ControlPoints);
            }
            else if (SingletonScript.Instance.OptionChange == 1)
            {//NLI Form
                point = NLI_Form(t, ControlPoints);
            }
            positions[i] = point;
        }
        GraphLine.SetPositions(positions);
    }


    //
    //BB Form - START
    Vector3 BB_Form(float t, List<Vector3> points)
    {
        n = points.Count - 1;
        value = Vector3.zero;
        for (int i = 0; i <= n; i++)
        {
            bernstein = BernsteinPolynomial(i, n, t);
            value += bernstein * points[i];
        }
        return value;
    }

    float BernsteinPolynomial(int i, int n, float t)
    {
        return CalculateBinomialCoeff(n, i) * Mathf.Pow(1 - t, n - i) * Mathf.Pow(t, i);

    }
    //BB Form - END
    //

    //
    //NLI Form - START
    Vector3 NLI_Form(float t, List<Vector3> points)
    {
        if (points.Count == 1)
            return points[0];

        List<Vector3> newPoints = new List<Vector3>();
        for (int i = 0; i < points.Count - 1; i++)
        {
            Vector3 point = (1 - t) * points[i] + t * points[i + 1];
            newPoints.Add(point);
        }
        return NLI_Form(t, newPoints);
    }
    //NLI Form - END
    //



    int CalculateBinomialCoeff(int n, int i)
    {
        if (i == 0 || i == n)
        {
            return 1;
        }

        return CalculateBinomialCoeff(n - 1, i - 1) + CalculateBinomialCoeff(n - 1, i);
    }


}
