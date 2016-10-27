#include "Visuals.h"
#include "TabPanel.h"

RotationZ::RotationZ()
	: Visuals()
	, isOrg(false)
	, org(0,0)
{
}

void RotationZ::DrawVisual(int time)
{
	if(time != oldtime && tbl[6]>3){
		from=CalcMovePos();
		from.x/=wspw; from.y/=wsph;
		to=from;
		if(org==from){org=from;}
		else{
			to=org;
		}
	}
	float rad =0.01745329251994329576923690768489f;
	float radius= sqrt(pow(abs(org.x - from.x),2) + pow(abs(org.y - from.y),2)) +40;
	D3DXVECTOR2 v2[6];
	VERTEX v5[726];
	CreateVERTEX(&v5[0], org.x, org.y + (radius + 10.f), 0xAA121150);
	CreateVERTEX(&v5[1], org.x, org.y + radius, 0xAA121150);
	for(int j=0; j<181; j++){
		float xx= org.x + ((radius + 10.f) * sin ( (j*2) * rad ));
		float yy= org.y + ((radius + 10.f) * cos ( (j*2) * rad ));
		float xx1= org.x + (radius * sin ( (j*2) * rad ));
		float yy1= org.y + (radius * cos ( (j*2) * rad ));
		CreateVERTEX(&v5[j + 364], xx, yy, 0xAAFF0000);
		CreateVERTEX(&v5[j + 545], xx1, yy1, 0xAAFF0000);
		if(j<1){continue;}
		CreateVERTEX(&v5[(j*2)], xx, yy, 0xAA121150);
		CreateVERTEX(&v5[(j*2)+1], xx1, yy1, 0xAA121150);
		
	}
	if(radius){
		float xx1= org.x + ((radius-40) * sin ( lastmove.y * rad ));
		float yy1= org.y + ((radius-40) * cos ( lastmove.y * rad ));
		v2[0].x=xx1-5.0f;
		v2[0].y=yy1;
		v2[1].x=xx1+5.0f;
		v2[1].y=yy1;
		v2[2]=org;
		v2[3].x=xx1;
		v2[3].y=yy1;
		float xx2= xx1 + (radius * sin ( (lastmove.y+90) * rad ));
		float yy2= yy1 + (radius * cos ( (lastmove.y+90) * rad ));
		float xx3= xx1 + (radius * sin ( (lastmove.y-90) * rad ));
		float yy3= yy1 + (radius * cos ( (lastmove.y-90) * rad ));
		v2[4].x=xx2;
		v2[4].y=yy2;
		v2[5].x=xx3;
		v2[5].y=yy3;
		line->SetWidth(10.f);
		line->Begin();
		line->Draw(v2,2,0xAAFF0000);
		line->End();
		line->SetWidth(2.f);
		line->Begin();
		line->Draw(&v2[2],2,0xFFBB0000);
		line->Draw(&v2[4],2,0xFFBB0000);
		line->End();
	}
	v2[0]=org;
	v2[1]=to;
	v2[2].x=org.x-10.0f;
	v2[2].y=org.y;
	v2[3].x=org.x+10.0f;
	v2[3].y=org.y;
	v2[4].x=org.x;
	v2[4].y=org.y-10.0f;
	v2[5].x=org.x;
	v2[5].y=org.y+10.0f;
	line->SetWidth(5.f);

	HRN(device->DrawPrimitiveUP( D3DPT_TRIANGLESTRIP, 360, v5, sizeof(VERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 180, &v5[364], sizeof(VERTEX) ),"primitive failed");
	HRN(device->DrawPrimitiveUP( D3DPT_LINESTRIP, 180, &v5[545], sizeof(VERTEX) ),"primitive failed");
	line->SetWidth(2.f);
	line->Begin();
	line->Draw(&v2[2],2,0xFFBB0000);
	line->End();
	line->Begin();
	line->Draw(&v2[4],2,0xFFBB0000);
	line->End();
	line->Begin();
	line->Draw(&v2[0],2,0xFFBB0000);
	line->End();

	
}

wxString RotationZ::GetVisual()
{
	if(isOrg){
		return "\\org("+getfloat(org.x*wspw)+","+getfloat(org.y*wsph)+")";
	}

	float angle = lastmove.x - atan2((org.y-to.y), (org.x-to.x)) * (180.f / 3.1415926536f);
	angle = fmodf(angle + 360.f, 360.f);
	lastmove.y = angle;
	
	return "\\frz"+ getfloat(angle);
}

void RotationZ::OnMouseEvent(wxMouseEvent &evt)
{
	if(blockevents){return;}
	bool click = evt.LeftDown()||evt.RightDown()||evt.MiddleDown();
	bool holding = (evt.LeftIsDown()||evt.RightIsDown()||evt.MiddleIsDown());
	
	int x, y;
	if(tab->Video->isfullskreen){wxGetMousePosition(&x,&y);}
	else{evt.GetPosition(&x,&y);}

	if(evt.ButtonUp()){
		if(tab->Video->HasCapture()){tab->Video->ReleaseMouse();}
		SetVisual(GetVisual(),false,(isOrg)?100:0);
		to=org;
		if(isOrg){
			lastmove.x = atan2((org.y-y), (org.x-x)) * (180.f / 3.1415926536f);
			lastmove.x+=lastmove.y;
		}
		tab->Video->Render();
		if(!hasArrow){tab->Video->SetCursor(wxCURSOR_ARROW);hasArrow=true;}
		isOrg=false;
	}

	if(click){
		tab->Video->CaptureMouse();
		tab->Video->SetCursor(wxCURSOR_SIZING);
		hasArrow=false;
		if(abs(org.x-x)<8 && abs(org.y-y)<8){
			isOrg=true;
			diffs.x=org.x-x;
			diffs.y=org.y-y;
			return;
		}else{
			lastmove.x = atan2((org.y-y), (org.x-x)) * (180.f / 3.1415926536f);
			lastmove.x+=lastmove.y;
		}
	}else if(holding){
		if(isOrg){
			org.x = x+diffs.x;
			org.y = y+diffs.y;
			
			SetVisual(GetVisual(),true,100);//type tak�e ma liczb� 100 by by�o rozpoznawalne.
			return;
		}
		to.x=x;to.y=y;
		SetVisual(GetVisual(),true,0);

	}

}

void RotationZ::SetCurVisual()
{
	D3DXVECTOR2 linepos = GetPosnScale(NULL, NULL, tbl);
	if(tbl[6]>3){linepos=CalcMovePos();}
	from = D3DXVECTOR2(linepos.x/wspw,linepos.y/wsph);
	lastmove = D3DXVECTOR2(0, 0);
	wxString res;
	if(tab->Edit->FindVal("frz?([^\\\\}]+)", &res)){
		double result=0; res.ToDouble(&result);
		lastmove.y=result;
		lastmove.x+=lastmove.y;
	}
	if(tab->Edit->FindVal("org\\(([^\\)]+)", &res)){
		wxString rest;
		double orx,ory;
		if(res.BeforeFirst(',',&rest).ToDouble(&orx)){org.x=orx/wspw;}
		if(rest.ToDouble(&ory)){org.y=ory/wspw;}
	//wxLogStatus("%f %f", orx,ory);
	}else{org=from;}
	to=org;

}