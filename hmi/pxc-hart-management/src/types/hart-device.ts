import { hartServerUrl } from '.'
import axios from 'axios'
import { HartVars, HartVarsDto } from './hart-vars'

export interface HartDeviceDto {
    name: string
    company: string
    longTag: string
    cmdRevLvl: number
    manufacturerId: number
    privateLabelDistCode: number
    profile: number
    revision: number
    ioCard: number
    channel: number
    vars?: HartVars
}

export const verboseHartDeviceAttrs = {
    name: "Name",
    company: "Company",
    longTag: "Long Tag",
    cmdRevLvl: "HART Revision",
    revision: "Device Revision",
    ioCard: "I/O Card",
    channel: "Channel"
}

export class HartDevice implements HartDeviceDto {
    name: string
    company: string
    longTag: string
    cmdRevLvl: number
    manufacturerId: number
    privateLabelDistCode: number
    profile: number
    revision: number
    ioCard: number
    channel: number
    _vars?: HartVars

    constructor(data: HartDeviceDto) {
        this.name = data.name
        this.company = data.company
        this.longTag = data.longTag
        this.cmdRevLvl = data.cmdRevLvl
        this.manufacturerId = data.manufacturerId
        this.privateLabelDistCode = data.privateLabelDistCode
        this.profile = data.profile
        this.revision = data.revision
        this.ioCard = data.ioCard
        this.channel = data.channel
        this._vars = data.vars
    }

    public async getVars(): Promise<HartVars> {
        if (this._vars) { // already have vars loaded
            return new Promise(() => this._vars)
        } else { // go get the vars
            const dummyVar = {
                value: 0,
                units: '',
                lastUpdated: new Date()
            }
            this._vars = {
                loopCurrent: dummyVar,
                pv: dummyVar
            }
            const gwSN = ''
            const url = hartServerUrl + '/gw/' + gwSN + '/vars/' + this.ioCard + '/' + this.channel 
            await axios.get(url).then(res => {
                const varsDto = res.data as HartVarsDto
                Object.keys(varsDto).forEach(key => {
                    // these keys are by definition allowed on HartVars objects, but TS won't infer that
                    (this._vars as any)[key].value = (varsDto as any)[key].value
                    (this._vars as any)[key].units = (varsDto as any)[key].units
                    (this._vars as any)[key].lastUpdated = new Date()
                })
            })
            return this._vars
        }
    }
}