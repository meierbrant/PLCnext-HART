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
    extendedDeviceStatus: hart_extended_device_status_info[]
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
    extendedDeviceStatus: hart_extended_device_status_info[]
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
        this.extendedDeviceStatus = data.extendedDeviceStatus
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
                // FIXME
                const varsDto = res.data as HartVarsDto as any
                const vars = this._vars as any
                Object.keys(varsDto).forEach(key => {
                    // these keys are by definition allowed on HartVars objects, but TS won't infer that
                    vars[key].value = varsDto[key].value
                    vars[key].units = varsDto[key].units
                    vars[key].lastUpdated = new Date()
                })
                this._vars = vars
            })
            return this._vars
        }
    }
}

export type hart_extended_device_status_info = {
    name: string,
    description: string,
    condensedStatus: boolean
}

export type hart_command_brief = {
    "number": number,
    "description": string
}

export type hart_command_data_field = {
    byte: string | number,
    format: string,
    description: string,
    value: any
}

export type hart_command_status = {
    class: "success" | "error" | "warning",
    description: string
}

export type hart_command_response = {
    number: number,
    description: string,
    data: hart_command_data_field[],
    status: hart_command_status
}

export type hart_log_var = {
    timestamp: string,
    units: string,
    value: number
}

export type hart_device_vars_log = {
    pv: hart_log_var[],
    sv: hart_log_var[],
    tv: hart_log_var[],
    qv: hart_log_var[]
}